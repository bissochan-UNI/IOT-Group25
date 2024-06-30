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

Graphics_Context g_sContext;

#define NUM_OPTIONS 5
#define BUTTON_NEXT BIT1
#define BUTTON_SELECT BIT4

#define LIGHT_THRESHOLD 800.0 // Light threshold to consider the light on
#define TEMP_THRESHOLD 10.0   // Temperature threshold for significant change

const char *menuOptions[NUM_OPTIONS] = {"Toilet", "Washing Hands", "Shower", "Make Up", "Skincare"};
const int menuTimes[NUM_OPTIONS] = {5, 2, 10, 8, 7};

// Function to display menu and handle user selections
int menu() {
    int currentOption = 0;
    int selectedOptions[NUM_OPTIONS] = {0};
    int totalTime = 0;
    int selecting = 1;
    int buttonPressed;

    while (selecting) {
        displayMessage(menuOptions[currentOption]); // Display current menu option

        // Wait for button press
        buttonPressed = 0;
        while (!buttonPressed) {
            if (!(P1->IN & BUTTON_NEXT)) {
                // Debounce
                while (!(P1->IN & BUTTON_NEXT));
                buttonPressed = 1;
                currentOption = (currentOption + 1) % NUM_OPTIONS; // Move to next option
            } else if (!(P1->IN & BUTTON_SELECT)) {
                // Debounce
                while (!(P1->IN & BUTTON_SELECT));
                buttonPressed = 1;
                if (!selectedOptions[currentOption]) {
                    selectedOptions[currentOption] = 1;
                    totalTime += menuTimes[currentOption]; // Add selected time

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
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // Disable watchdog timer

    // Initialize peripherals
    temp_sensor_init();
    lux_sensor_init();
    buzzerInit();
    initDisplay();
    configureTimer(); // Configure timer
    clearDisplay();

    char buffer[100];

    int lightOn = 0;
    int timerA0Started = 0;
    int timerA1Started = 0;
    float initialTemp = 0;

    displayMessage("Programma Attivato!"); // Display activation message

    float startingLightLevel = read_lux(); // Read initial light level
    sprintf(buffer, " Starting Lux: %f", startingLightLevel);
    displayMessage(buffer);

    int totalSelectedTime = menu(); // Display menu and get total selected time
    setTimer(totalSelectedTime); // Set the timer with total selected time

    while (1) {
        float lightLevel = read_lux(); // Read current light level
        sprintf(buffer, "Lux: %f", lightLevel);
        displayMessage(buffer);
        memset(buffer, 0, sizeof(buffer));

        if (lightLevel > LIGHT_THRESHOLD) {
            if (!lightOn) { // If light is on
                displayMessage("Luce Accesa");
                lightOn = 1;
                initialTemp = read_temp(); // Read initial temperature
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, " Temp %f", initialTemp);
                displayMessage(buffer);
                if (!timerA0Started) {
                    timerA0Started = 1; // Start timer A0
                    displayMessage("Starto timer A0");
                    startTimerA0();
                }
            } else {
                int currentTemp = read_temp();
                if (currentTemp > initialTemp) { // If current temperature is higher than initial temperature
                    displayMessage("Stoppo timer A0, sopra soglia");
                    memset(buffer, 0, sizeof(buffer));
                    sprintf(buffer, "CurrentTemp %f", initialTemp);
                    displayMessage(buffer);
                    stopTimerA0();
                    if (!timerA1Started) {
                        timerA1Started = 1;
                        startTimerA1(); // Start timer A1
                        displayMessage("Starto timer A1");
                    }
                }
            }
        } else {
            if (lightOn) { // If light is off
                lightOn = 0;
                timerA0Started = 0;
                stopTimerA0(); // Stop timer A0
                stopTimerA1(); // Stop timer A1
                displayMessage("Stoppo timer A0, sotto soglia");
                turnOffRedLED(); // Turn off red LED
                clearDisplay(); // Clear display message
                stopBuzzerTimer(); // Stop buzzer timer
                turnOffBuzzer(); // Turn off buzzer
            }
        }

        if (timerA0Started && timerA0Expired) {
            turnOnRedLED();
            displayMessage("Timer A0 SCADUTO");
            startBuzzerTimer(); // Start buzzer timer
        }

        if (timerA1Started) {
            int currentTemp = read_temp();
            if (currentTemp <= initialTemp - TEMP_THRESHOLD) {
                stopTimerA1();
                timerA1Started = 0;
                startTimerA0(); // Resume timer A0 from where it was stopped
            } else if (timerA1Expired) {
                turnOnRedLED();
                displayMessage("Temp high - timer expired");
                startBuzzerTimer(); // Start buzzer timer
            }
        }

        //__sleep();
        __no_operation(); // For debugger
    }
}
