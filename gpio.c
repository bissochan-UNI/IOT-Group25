// gpio.c

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

void temp_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    TMP006_init();
    __delay_cycles(100000);
}

void lux_sensor_init(){
    Init_I2C_GPIO();
    I2C_init();
    OPT3001_init();
    __delay_cycles(100000);
}

/* Timer_A Compare Configuration Parameter  (PWM) */
Timer_A_CompareModeConfig compareConfig_PWM = {
        TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR3
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        10000                                        // 25% Duty Cycle initially
        };

/* Timer_A Up Configuration Parameter */
const Timer_A_UpModeConfig upConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,                      // SMCLK = 3 MhZ
        TIMER_A_CLOCKSOURCE_DIVIDER_12,         // SMCLK/12 = 250 KhZ
        20000,                                  // 40 ms tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
        };

void buzzerInit()
{
    /* Configures P2.7 to PM_TA0.4 for using Timer PWM to control the buzzer */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7,
    GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A0 for Up Mode and starting */
    Timer_A_configureUpMode(TIMER_A3_BASE, &upConfig);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_UP_MODE);

    /* Initialize compare registers to generate PWM */
    Timer_A_initCompare(TIMER_A3_BASE, &compareConfig_PWM); // For P2.7
}

float read_temp(){
    float temp = TMP006_getTemp();
    return temp;
}

float read_lux(){
    float lux = OPT3001_getLux();
    return lux;
}

void turnOnRedLED(void) {
    P2OUT |= BIT0;
}

void turnOffRedLED(void) {
    P2OUT &= ~BIT0;
}

void turnOnBuzzer(void) {
    P2OUT |= BIT1;
}

void turnOffBuzzer(void) {
    P2OUT &= ~BIT1;
}

void initDisplay(void) {
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

void clearDisplay(void){
    Graphics_clearDisplay(&g_sContext);
}

void displayMessage(const char* message) {
    clearDisplay();
    Graphics_drawStringCentered(&g_sContext, (int8_t *) message, strlen(message), 64, 64, OPAQUE_TEXT);
    __delay_cycles(3000000);
}

void configurePorts() {
    /* Configure P2.0, P2.1, P2.2 as GPIO outputs */
    P2->SEL0 &= ~(BIT0 | BIT1 | BIT2);
    P2->SEL1 &= ~(BIT0 | BIT1 | BIT2);
    P2->DIR |= (BIT0 | BIT1 | BIT2);
    P2->OUT &= ~(BIT0 | BIT1 | BIT2);

    /* Configure P1.1 and P1.4 as GPIO inputs with pull-up resistors */
    P1->SEL0 &= ~(BIT1 | BIT4);
    P1->SEL1 &= ~(BIT1 | BIT4);
    P1->DIR &= ~(BIT1 | BIT4);
    P1->OUT |= (BIT1 | BIT4);
    P1->REN |= (BIT1 | BIT4);
}
