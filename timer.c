#include "msp.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "Drivers/Crystalfontz128x128_ST7735.h"
#include "Drivers/HAL_I2C.h"
#include "Drivers/HAL_TMP006.h"
#include "Drivers/HAL_OPT3001.h"

/* Graphic library context */
extern Graphics_Context g_sContext;

// Initialize temperature sensor
void temp_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    TMP006_init();
    __delay_cycles(100000);
}

// Initialize light sensor
void lux_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    OPT3001_init();
    __delay_cycles(100000);
}

/* Timer_A Compare Configuration Parameter (PWM) */
Timer_A_CompareModeConfig compareConfig_PWM = {
    TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR4
    TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
    TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
    10000                                       // 25% Duty Cycle initially
};

/* Timer_A Up Configuration Parameter */
const Timer_A_UpModeConfig upConfig = {
    TIMER_A_CLOCKSOURCE_SMCLK,                  // SMCLK = 3 MHz
    TIMER_A_CLOCKSOURCE_DIVIDER_12,             // SMCLK/12 = 250 kHz
    20000,                                      // 40 ms tick period
    TIMER_A_TAIE_INTERRUPT_DISABLE,             // Disable Timer interrupt
    TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,        // Disable CCR0 interrupt
    TIMER_A_DO_CLEAR                            // Clear value
};

// Initialize buzzer
void buzzerInit() {
    // Configures P2.7 to PM_TA0.4 for using Timer PWM to control the buzzer
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    // Configuring Timer_A0 for Up Mode and starting
    Timer_A_configureUpMode(TIMER_A3_BASE, &upConfig);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_UP_MODE);

    // Initialize compare registers to generate PWM
    Timer_A_initCompare(TIMER_A3_BASE, &compareConfig_PWM); // For P2.7
}

// Read temperature
float read_temp() {
    return TMP006_getTemp();
}

// Read light intensity
float read_lux() {
    return OPT3001_getLux();
}

// Turn on red LED
void turnOnRedLED() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

// Turn off red LED
void turnOffRedLED() {
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

// Turn on buzzer
void turnOnBuzzer() {
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 5000);
}

// Turn off buzzer
void turnOffBuzzer() {
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 0);
}

// Initialize display
void initDisplay() {
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_YELLOW);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
}

// Clear display
void clearDisplay() {
    Graphics_clearDisplay(&g_sContext);
}

// Display message on screen
void displayMessage(const char *message) {
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) message, AUTO_STRING_LENGTH, 64, 64, OPAQUE_TEXT);
}

// Configure GPIO ports
void configurePorts() {
    // Configure P1.0 as output (LED)
    P1->DIR |= BIT0;
    P1->OUT &= ~BIT0;

    // Configure P1.1 and P1.4 as input (buttons)
    P1->DIR &= ~(BIT1 | BIT4);
    P1->REN |= BIT1 | BIT4;
    P1->OUT |= BIT1 | BIT4;
}
