#pragma once

#include "RBControl_serialPWM.hpp"
#include <Arduino.h>

namespace TR {

const uint8_t PWM_CHANNELS = 32;
const uint16_t PWM_FREQUENCY = 330;
extern const uint8_t PWM_MAX;

const uint8_t REG_DAT = 2;
const uint8_t REG_CLK = 14;
const uint8_t REG_LATCH = 12;
const uint8_t I2C_SDA = 15;
const uint8_t I2C_SCL = 13;
const uint8_t LIGHT_PIN = 4;
const uint8_t LIGHT_PWM_CHANNEL = 0;
const uint16_t LIGHT_PWM_FREQUENCY = 5000;
const uint8_t LIGHT_PWM_RESOLUTION = 8;

extern rb::SerialPWM serialPWM;
extern int8_t pwm_index[32];

void setPWM(rb::SerialPWM::value_type& channel, int8_t power);
void updatePWM(void * param);
}

enum shiftRegPins {
    TR_OUT1, TR_OUT2, TR_OUT3, TR_OUT4, TR_OUT5, TR_OUT6, TR_OUT7, TR_OUT8, TR_OUT9, TR_OUT10, TR_OUT11, TR_OUT12, TR_OUT13, TR_OUT14, TR_OUT15, TR_OUT16, TR_OUT17, TR_OUT18, TR_OUT19, TR_OUT20, TR_OUT21, TR_OUT22, TR_OUT23, TR_OUT24, TR_OUT25, TR_OUT26, TR_OUT27, TR_OUT28, TR_OUT29, TR_OUT30, TR_OUT31, TR_OUT32, TR_OUT_LIGHT
};

void digitalWrite(shiftRegPins pin, const bool state);
bool digitalRead();
void analogWrite(shiftRegPins pin, const int8_t value);

class TrackRayClass {
    bool buttonPressed = 0;
    uint8_t lightBrightness = 0;
public:
    TrackRayClass();
    bool getButton();
    void setButton(bool pressed);
    void setFlashLight(int16_t brightness);
    void begin();
};

extern TrackRayClass TrackRay;
