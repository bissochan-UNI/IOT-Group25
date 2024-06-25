//
// Created by lucab on 27/05/2024.
//

#ifndef BATHCONTROLLER_GPIO_H
#define BATHCONTROLLER_GPIO_H

#include <stdint.h>

// Function declarations
void temp_sensor_init(void);
void lux_sensor_init(void);
void buzzerInit(void);
float read_temp(void);
float read_lux(void);
void turnOnRedLED(void);
void turnOffRedLED(void);
void turnOnBuzzer(void);
void turnOffBuzzer(void);
void initDisplay(void);
void clearDisplay(void);
void displayMessage(const char* message);
void configurePorts(void);


#endif //BATHCONTROLLER_GPIO_H
