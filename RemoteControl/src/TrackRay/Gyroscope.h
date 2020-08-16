#ifndef _Gyroscope_H_
#define _Gyroscope_H_

#include <Arduino.h>

#define INTERRUPT_PIN D0 //on MainBoard routed D3

bool initiateGyroscope(const int16_t aOffsets[]);
void updateGyroData(float * ypr);

#endif /*_Gyroscope_H_*/