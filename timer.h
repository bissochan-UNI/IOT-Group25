//
// Created by lucab on 27/05/2024.
//

#ifndef BATHCONTROLLER_TIMER_H
#define BATHCONTROLLER_TIMER_H

#include "msp.h"

void configureTimer(void);
void startTimerA0(void);
void stopTimerA0(void);
void resetTimerA0(void);
void startTimerA1(void);
void stopTimerA1(void);
void startBuzzerTimer(void);
void stopBuzzerTimer(void);
extern volatile int timerA0Expired;
extern volatile int timerA1Expired;
extern volatile int buzzerState;

#endif //BATHCONTROLLER_TIMER_H
