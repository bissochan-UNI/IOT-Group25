#include "msp.h"
#include "gpio.h"
#include "timer.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "Drivers/Crystalfontz128x128_ST7735.h"
#include "Drivers/HAL_I2C.h"
#include "timer.h"
#include "gpio.h"
Graphics_Context g_sContext;


#define NUM_OPTIONS 5
#define BUTTON_NEXT BIT1
#define BUTTON_SELECT BIT4

#define LIGHT_THRESHOLD 800.0 // Soglia di luminosità per considerare la luce accesa
#define TEMP_THRESHOLD 10.0    // Soglia di temperatura per considerare un cambiamento significativo


const char *menuOptions[NUM_OPTIONS] = {"Toilet", "Washing Hands", "Shower", "Make Up", "Skincare"};
const int menuTimes[NUM_OPTIONS] = {5, 2, 10, 8, 7};

int menu() {
    int currentOption = 0;
    int selectedOptions[NUM_OPTIONS] = {0};
    int totalTime = 0;
    int selecting = 1;
    int buttonPressed;

    while (selecting) {
        displayMessage(menuOptions[currentOption]);

        // Wait for button press
        buttonPressed = 0;
        while (!buttonPressed) {
            if (!(P1->IN & BUTTON_NEXT)) {
                // Debounce
                while (!(P1->IN & BUTTON_NEXT));
                buttonPressed = 1;
                currentOption = (currentOption + 1) % NUM_OPTIONS;
            } else if (!(P1->IN & BUTTON_SELECT)) {
                // Debounce
                while (!(P1->IN & BUTTON_SELECT));
                buttonPressed = 1;
                if (!selectedOptions[currentOption]) {
                    selectedOptions[currentOption] = 1;
                    totalTime += menuTimes[currentOption];

                    // Ask if they want to select another option
                    displayMessage("Select another? Y/N");
                    while (!buttonPressed) {
                        if (!(P1->IN & BUTTON_NEXT)) {
                            // Debounce
                            while (!(P1->IN & BUTTON_NEXT));
                            buttonPressed = 1;
                            selecting = 0; // No more selection
                        } else if (!(P1->IN & BUTTON_SELECT)) {
                            // Debounce
                            while (!(P1->IN & BUTTON_SELECT));
                            buttonPressed = 1;
                            // Continue selecting
                        }
                    }
                }
            }
        }
    }

    return totalTime;
}

void main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // Disabilita il watchdog timer

    temp_sensor_init();
    lux_sensor_init();
    buzzerInit();
    initDisplay();
    configureTimer(); // Configura il timer
    clearDisplay();

    char buffer[100];

    int lightOn = 0;
    int timerA0Started = 0;
    int timerA1Started = 0;
    float initialTemp = 0;

    displayMessage("Programma Attivato!");



    float startingLightLevel = read_lux();
    sprintf(buffer, " Starting Lux: %f", startingLightLevel);
    displayMessage(buffer);

    int totalSelectedTime = menu();
    setTimer(totalSelectedTime); 

    while (1) {
            float lightLevel = read_lux();
            sprintf(buffer, "Lux: %f", lightLevel);
            displayMessage(buffer);
            memset(buffer, 0, sizeof(buffer));
            if (lightLevel > LIGHT_THRESHOLD) {
                if (!lightOn) { // SETTO LUCE ACCESA
                    displayMessage("Luce Accesa");
                    lightOn = 1;
                    initialTemp = read_temp();
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, " Temp %f", initialTemp);
                    displayMessage(buffer);
                    if (!timerA0Started) {
                        timerA0Started = 1;                     // Starto primo timer
                        displayMessage("Starto timer A0");
                        startTimerA0();
                    }
                } else {  // LUCE GIA ACCESA
                    int currentTemp = read_temp();
                    if (currentTemp > initialTemp) {    //TEMPERATURA PIU CALDA DI PRIMA, STOPPO TIMER
                        displayMessage("Stoppo timer A0, sopra soglia");
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "CurrentTemp %f", initialTemp);
                        displayMessage(buffer);
                        stopTimerA0();
                        if (!timerA1Started) {
                            timerA1Started = 1;
                            startTimerA1(); // Inizia il timer A1
                            displayMessage("Starto timer A1");
                        }
                    }
                }
            } else { // Se luce SOTTO SOGLIA
                if (lightOn) {  //se era settata accesa, la spengo
                    lightOn = 0;
                    timerA0Started = 0;
                    stopTimerA0(); // Ferma il timer A0
                    stopTimerA1();
                    displayMessage("Stoppo timer A0, sotto soglia");
                    turnOffRedLED(); // Spegne il LED rosso
                    clearDisplay();// Cancella il messaggio sul display
                    stopBuzzerTimer(); // Ferma il buzzer
                    turnOffBuzzer(); // Spegne il buzzer
                }
            }

            if (timerA0Started && timerA0Expired) {
                turnOnRedLED();
                displayMessage("Timer A0 SCADUTO");
                startBuzzerTimer(); // Inizia il timer per il bip del buzzer
            }

            if (timerA1Started) {
                int currentTemp = read_temp();
                if (currentTemp <= initialTemp - TEMP_THRESHOLD) {
                    stopTimerA1();
                    timerA1Started = 0;
                    startTimerA0(); // Riprende il timer A0 da dove era stato fermato
                } else if (timerA1Expired) {
                    turnOnRedLED();
                    displayMessage("Temp high - timer expired");
                    startBuzzerTimer(); // Inizia il timer per il bip del buzzer
                }
            }

            //__sleep();
            __no_operation(); // Per debugger
        }
}
