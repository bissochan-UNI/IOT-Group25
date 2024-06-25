#include "timer.h"
#include "gpio.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "Drivers/Crystalfontz128x128_ST7735.h"
#include "Drivers/HAL_I2C.h"

/* Graphic library context */
Graphics_Context g_sContext;

volatile int timerA0Expired = 0;
volatile int timerA1Expired = 0;
volatile int buzzerState = 0;

void configureTimer(void) {
    // Configura il timer A0
    //TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK; // Usa SMCLK
    //TIMER_A0->CTL |= TIMER_A_CTL_MC__STOP; // Modalità stop per configurazione
    //TIMER_A0->CTL |= TIMER_A_CTL_ID_3; // Usa un prescaler per dividere il clock per 8
    //TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // Abilita l'interruzione per CCR0
    //TIMER_A0->CCR[0] = 100; // 37500 = 5 minuti (3 MHz / 8 / 50000 = 7.5 Hz, 5 * 60 * 7.5)
    //NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31); // Abilita IRQ per TA0

    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID__8; // Use SMCLK, Up mode, divide by 8
    TIMER_A0->CCR[0] = 65535; // Max value for CCR0
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // Enable interrupt for CCR0
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31); // Enable IRQ for TA0


    // Configura il timer A1
    TIMER_A1->CTL = TIMER_A_CTL_SSEL__SMCLK; // Usa SMCLK
    TIMER_A1->CTL |= TIMER_A_CTL_MC__STOP; // Modalità stop per configurazione
    TIMER_A1->CTL |= TIMER_A_CTL_ID_3; // Usa un prescaler per dividere il clock per 8
    TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE; // Abilita l'interruzione per CCR0
    TIMER_A1->CCR[0] = 22500; // 3 minuti (3 MHz / 8 / 50000 = 7.5 Hz, 3 * 60 * 7.5)
    NVIC->ISER[0] = 1 << ((TA1_0_IRQn) & 31); // Abilita IRQ per TA1

    // Configura il timer A2 per il buzzer
    TIMER_A2->CTL = TIMER_A_CTL_SSEL__SMCLK; // Usa SMCLK
    TIMER_A2->CTL |= TIMER_A_CTL_MC__STOP; // Modalità stop per configurazione
    TIMER_A2->CTL |= TIMER_A_CTL_ID_3; // Usa un prescaler per dividere il clock per 8
    TIMER_A2->CCTL[0] = TIMER_A_CCTLN_CCIE; // Abilita l'interruzione per CCR0
    TIMER_A2->CCR[0] = 9375; // Bip ogni 1 secondo (3 MHz / 8 / 9375 = 40 Hz, 0.5s on, 0.5s off)
    NVIC->ISER[0] = 1 << ((TA2_0_IRQn) & 31); // Abilita IRQ per TA2

}

void startTimerA0(void) {
    timerA0Expired = 0;
    TIMER_A0->CTL |= TIMER_A_CTL_CLR; // Resetta il timer
    TIMER_A0->CTL |= TIMER_A_CTL_MC__UP; // Modalità up
}

void stopTimerA0(void) {
    TIMER_A0->CTL &= ~TIMER_A_CTL_MC_0; // Ferma il timer
}

void resetTimerA0(void) {
    stopTimerA0();
    startTimerA0();
}

void startTimerA1(void) {
    timerA1Expired = 0;
    TIMER_A1->CTL |= TIMER_A_CTL_CLR; // Resetta il timer
    TIMER_A1->CTL |= TIMER_A_CTL_MC__UP; // Modalità up
}

void stopTimerA1(void) {
    TIMER_A1->CTL &= ~TIMER_A_CTL_MC_0; // Ferma il timer
}

void startBuzzerTimer(void) {
    buzzerState = 0;
    TIMER_A2->CTL |= TIMER_A_CTL_CLR; // Resetta il timer
    TIMER_A2->CTL |= TIMER_A_CTL_MC__UP; // Modalità up
}

void stopBuzzerTimer(void) {
    TIMER_A2->CTL &= ~TIMER_A_CTL_MC_0; // Ferma il timer
}



/* Handler dell'interruzione del timer A0 */
//void TA0_0_IRQHandler(void) {
    // Pulisce il flag di interruzione
    //TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    // Segna che il timer è scaduto
    //timerA0Expired = 1;
//}

volatile uint32_t overflow_count = 0;

void TA0_0_IRQHandler(void) {
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // Clear interrupt flag
    overflow_count++;
    if (overflow_count >= 1717) {
        // 5 minutes elapsed
        overflow_count = 0; // Reset for next time if needed
        // Perform desired action here
    }
}


/* Handler dell'interruzione del timer A1 */
void TA1_0_IRQHandler(void) {
    // Pulisce il flag di interruzione
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    // Segna che il timer è scaduto
    timerA1Expired = 1;
}

/* Handler dell'interruzione del timer A2 */
void TA2_0_IRQHandler(void) {
    // Pulisce il flag di interruzione
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    // Alterna lo stato del buzzer
    buzzerState = !buzzerState;
    if (buzzerState) {
        turnOnBuzzer();
    } else {
        turnOffBuzzer();
    }
}
