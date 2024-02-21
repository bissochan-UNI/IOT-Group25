#include "msp.h"
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "Drivers/Crystalfontz128x128_ST7735.h"
#include "Drivers/HAL_I2C.h"
#include "Drivers/HAL_TMP006.h"
#include "Drivers/HAL_OPT3001.h"

volatile int buttonPressed = 0; // Variabile globale per indicare se il pulsante è stato premuto
volatile int waitForButton = 1; // Variabile globale per indicare se il sistema deve aspettare il premere del pulsante
volatile int timerExpired = 0; // Variabile globale per indicare se il timer è scaduto
volatile uint32_t remainingTicks = 0;
volatile int startinglight = 0;
volatile int alarm_active = 0;
volatile char string[100];

#define RED_LED_PIN     BIT0 // Assumendo che la luce rossa sia collegata al pin P1.0
#define TIMER_INTERVAL_MS 1000 // Interval in milliseconds
#define PIN_NUMBER 2
#define FREQUENCY 3000000
#define UP_BUTTON       (P1IN & BIT1)
#define DOWN_BUTTON     (P1IN & BIT4)
#define CONFIRM_BUTTON  (P1IN & BIT2)

enum MenuOptions {
    OPTION_1,
    OPTION_2,
    OPTION_3,
    OPTION_4,
    OPTION_5,
    NUM_OPTIONS
};

/* Graphic library context */
Graphics_Context g_sContext;

void _graphicsInit()
{
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128, &g_sCrystalfontz128x128_funcs);
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
        Graphics_clearDisplay(&g_sContext);
}

const char *menuStrings[NUM_OPTIONS] = {
    "Option 1",
    "Option 2",
    "Option 3",
    "Option 4",
    "Option 5"
};

int selectedOption = 0;

void delay(int milliseconds) {
    int i;
    for (i = 0; i < milliseconds; ++i) {
        __delay_cycles(3000); // assuming 3MHz clock
    }
}

void displayMenu() {
    // Display current option
    // printf("%s\n", menuStrings[selectedOption]);
    sprintf(string, "%s\n", menuStrings[selectedOption]);
        Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)string,
                                        8,
                                        64,
                                        50,
                                        OPAQUE_TEXT);
}

void selectOption() {
    // Handle selection of option
    // Add code here to perform actions based on selected option
    // printf("Selected: %s\n", menuStrings[selectedOption]);
    sprintf(string, "Selected: %s\n", menuStrings[selectedOption]);
        Graphics_drawStringCentered(&g_sContext,
                                        (int8_t *)string,
                                        8,
                                        64,
                                        50,
                                        OPAQUE_TEXT);
}

int menu() {
    int repeat = 1;
    int i;

    while (repeat) {
        for (i = 0; i < NUM_OPTIONS; ++i) {
            selectedOption = i;
            displayMenu();

            // Wait for confirmation
            while (!CONFIRM_BUTTON) {
                // Check for button presses
                if (!UP_BUTTON && selectedOption > 0) {
                    selectedOption--;
                    while (!UP_BUTTON); // Wait for button release
                    displayMenu();
                } else if (!DOWN_BUTTON && selectedOption < NUM_OPTIONS - 1) {
                    selectedOption++;
                    while (!DOWN_BUTTON); // Wait for button release
                    displayMenu();
                }
            }

            selectOption();

            sprintf(string, "Do you want to select another option? (Press UP for Yes, DOWN for No): ", menuStrings[selectedOption]);
                Graphics_drawStringCentered(&g_sContext,
                                                (int8_t *)string,
                                                8,
                                                64,
                                                50,
                                                OPAQUE_TEXT);
            // printf("Do you want to select another option? (Press UP for Yes, DOWN for No): ");
                        while (!UP_BUTTON && !DOWN_BUTTON); // Wait for button press
                        if (UP_BUTTON) {
                            while (!UP_BUTTON); // Wait for button release
                        } else {
                            while (!DOWN_BUTTON); // Wait for button release
                            repeat = 0;
                            break;
                        }
        }
    }
    return 1;
}

void temp_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    TMP006_init();
    __delay_cycles(100000);
}

float read_temp(){
    float temp = TMP006_getTemp();
    return temp;
}

void lux_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    OPT3001_init();
    __delay_cycles(100000);
}

float read_lux(){
    float lux = OPT3001_getLux();
    return lux;
}

void configureButtonInterrupt() {
    // Configura il bottone come input
    P1->DIR &= ~(1 << PIN_NUMBER); // Imposta il pin come input
    P1->REN |= (1 << PIN_NUMBER); // Abilita la resistenza di pull-up o pull-down
    P1->OUT |= (1 << PIN_NUMBER); // Abilita la resistenza di pull-up

    // Configura l'interrupt sul fronte di salita (premere il pulsante)
    P1->IES |= (1 << PIN_NUMBER); // Imposta l'interrupt sul fronte di salita
    P1->IFG &= ~(1 << PIN_NUMBER); // Resetta il flag di interrupt per il pin
    P1->IE |= (1 << PIN_NUMBER); // Abilita l'interrupt per il pin
    NVIC_EnableIRQ(PORT1_IRQn); // Abilita l'interrupt controller per il port 1
}

void PORT1_IRQHandler() {
    // Controlla se l'interrupt è generato dal bottone
    if (P1->IFG & (1 << PIN_NUMBER)) {
        // Imposta la variabile globale per indicare che il pulsante è stato premuto
        buttonPressed = 1;

        // Cancella il flag di interrupt
        P1->IFG &= ~(1 << PIN_NUMBER);
    }
}

void startTimer(uint32_t seconds) {
    // Calcola il numero di tick del timer necessari per il tempo specificato
    uint32_t ticks = seconds * FREQUENCY; // FREQUENCY rappresenta la frequenza del timer

    // Salva il numero di tick rimanenti
    remainingTicks = ticks;

    // Configura il timer per generare un interrupt dopo il tempo specificato
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__ACLK | TIMER_A_CTL_MC__STOP; // Imposta il timer come fermo
    TIMER_A0->CCR[0] = ticks - 1; // Imposta il valore di confronto per generare l'interrupt
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // Abilita l'interrupt di confronto del canale 0
    TIMER_A0->CTL |= TIMER_A_CTL_MC__UP; // Avvia il timer in modalità di conteggio verso l'alto
}

int isTimerExpired() {
    return timerExpired;
}

void updateTimer(uint32_t seconds) {
    // Calcola il numero di tick del timer necessari per il tempo specificato
    uint32_t ticks = seconds * FREQUENCY; // FREQUENCY rappresenta la frequenza del timer

    // Aggiorna il numero di tick rimanenti
    remainingTicks = remainingTicks + ticks;

    // Aggiorna il valore di confronto del timer
    TIMER_A0->CCR[0] = ticks - 1;
}

// Gestore dell'interrupt del timer
void TA0_0_IRQHandler() {
    // Cancella il flag di interrupt
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    // Decrementa il numero di tick rimanenti
    remainingTicks--;

    // Verifica se il timer è scaduto
    if (remainingTicks == 0) {
        // Imposta la variabile globale per indicare che il timer è scaduto
        timerExpired = 1;
    }
}


int luciAccese(){
    float lux = read_lux();
    if (lux < startinglight+10.0){
        return 1;
    }
    return 0;
}

int rangeTemp(float temp){
    float temp2 = read_temp();
    if (temp2 < temp-0.5 || temp2 > temp+0.5){
        return 1;       //temperatura fuori range
    }
    return 0;           //temperatura in range
}

void TimerA0_Init(void) {
    TIMER_A0->CTL |= TIMER_A_CTL_CLR; // Clear timer
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | // Clock source: SMCLK
                    TIMER_A_CTL_MC__UP | // Up mode
                    TIMER_A_CTL_ID__1 | // Clock divider: 1
                    TIMER_A_CTL_IE; // Enable interrupts
    TIMER_A0->CCR[0] = TIMER_INTERVAL_MS * 1000; // Set compare value for desired interval
    NVIC_EnableIRQ(TA0_N_IRQn); // Enable Timer A0 interrupt
}

void TA0_N_IRQHandler(void) {
    // Your ISR code here
    // This function will be called every TIMER_INTERVAL_MS milliseconds

    // Toggle red LED
    P1->OUT |= RED_LED_PIN;

    // Display message on screen
    sprintf(string, "ALARM INTERRUPT TRIGGERED");
            Graphics_drawStringCentered(&g_sContext,
                                            (int8_t *)string,
                                            8,
                                            64,
                                            50,
                                            OPAQUE_TEXT);


    // Clear interrupt flag
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

void alarm(void) {
    // Se l'allarme è già attivo, disattivalo
    if (alarm_active) {
        alarm_active = 0;
        P1->OUT &= ~RED_LED_PIN; // Spegni la luce rossa
    } else {
        // Se l'allarme non è attivo, attivalo
        alarm_active = 1;
        P1->OUT |= RED_LED_PIN; // Accendi la luce rossa
         // Visualizza il messaggio di allarme sullo schermo
        sprintf(string, "ALARM FUNCTION TRIGGERED");
                    Graphics_drawStringCentered(&g_sContext,
                                                    (int8_t *)string,
                                                    8,
                                                    64,
                                                    50,
                                                    OPAQUE_TEXT);
    }
}

int main() {
    // Inizializza il sistema e le periferiche necessarie

    int tempo =0;
    float temperaturaIniziale = 0.0;
    startinglight = read_lux();
    // Configura l'interrupt del bottone
    configureButtonInterrupt();
    _graphicsInit();
    TimerA0_Init();
    __enable_irq();

    sprintf(string, "DIO PORCO");
                        Graphics_drawStringCentered(&g_sContext,
                                                        (int8_t *)string,
                                                        8,
                                                        64,
                                                        50,
                                                        OPAQUE_TEXT);

    while(1) {
        // Verifica se il sistema deve aspettare il pulsante o se deve eseguire myfunc()
        if (waitForButton) {
            // Metti il sistema in modalità di basso consumo fino a quando il pulsante non viene premuto
            __WFI();
        } else {
            // Esegui myfunc()
            tempo = menu();
            temperaturaIniziale = read_temp();
            while (!luciAccese){

            }

            startTimer(tempo);

            while(luciAccese && !isTimerExpired() && !rangeTemp(temperaturaIniziale)){

            }

            if(luciAccese && isTimerExpired()){
                alarm();
            }
            if (luciAccese && rangeTemp(temperaturaIniziale)){
                updateTimer(300);
            }
            while (luciAccese && !isTimerExpired()){

            }
            if(luciAccese && isTimerExpired()){
                alarm();
            }
            if(!luciAccese){
                sprintf(string, "SEI STATO BRAVO");
                            Graphics_drawStringCentered(&g_sContext,
                                                            (int8_t *)string,
                                                            8,
                                                            64,
                                                            50,
                                                            OPAQUE_TEXT);
            }



            // Riporta il sistema in attesa del pulsante
            waitForButton = 1;
        }
    }

    return 0;
}
