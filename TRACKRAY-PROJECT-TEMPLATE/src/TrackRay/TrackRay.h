#pragma once

#include "RBControl_serialPWM.hpp"
#include <Arduino.h>
#include "Preferences.h"

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
const float MOTOR_SPEED_FILTER_UPDATE_COEF = 0.15;
const char STORAGE_NAMESPACE[] = "trackray";

extern rb::SerialPWM serialPWM;
extern int8_t pwm_index[32];

void setPWM(rb::SerialPWM::value_type& channel, int8_t power);
void updatePWM(void * param);
}

enum shiftRegPins {
    TR_OUT1, TR_OUT2, TR_OUT3, TR_OUT4, TR_OUT5, TR_OUT6, TR_OUT7, TR_OUT8, 
    TR_OUT9, TR_OUT10, TR_OUT11, TR_OUT12, TR_OUT13, TR_OUT14, TR_OUT15, TR_OUT16, 
    TR_OUT17, TR_OUT18, TR_OUT19, TR_OUT20, TR_OUT21, TR_OUT22, TR_OUT23, TR_OUT24, 
    TR_OUT25, TR_OUT26, TR_OUT27, TR_OUT28, TR_OUT29, TR_OUT30, TR_OUT31, TR_OUT32,
};

bool trrReadButton();

void trrSetLedDigital(int8_t pin, const bool state);
void trrSetLedAnalog(int8_t pin, const int8_t value);
void trrSetLedAllDigital(const bool state);
void trrSetLedAllAnalog(const int8_t value);
void trrSetFlashLightDigital(const bool state);
void trrSetFlashLightAnalog(const int8_t value);

void trrMotorsSetSpeed(const int8_t speedLeft, const int8_t speedRight);
void trrMotorsSetSpeedLeft(const int8_t speed);
void trrMotorsSetSpeedRight(const int8_t speed);
void trrCanonSetSpeed(const int8_t speed);

bool trrGyroEnabled();
float trrGyroYaw();
float trrGyroPitch();
float trrGyroRoll();
void trrGyroData(float ypr[]);
void trrGyroCalibrate();

class TrackRayClass {
    bool buttonPressed = false;
    uint8_t lightBrightness = 0;
    int8_t motorsSpeed[3];
    float motorsSpeedFiltered[3];
    bool gyroEnabled = false;
    float gyroYPR[3];
    int16_t gyroOffsets[3];
    Preferences preferences;
public:
    TrackRayClass();
    bool getButton();
    void setButton(bool pressed);
    void setFlashLight(int16_t brightness);
    void setMotorsSpeed(const int8_t speed, const int8_t index);
    void updateMotorsSpeed();
    void begin();
    
    bool gyroGetEnabled();
    float gyroData(uint8_t index) ;
    void gyroCalibrate();
    void gyroUpdate();
    void printOffsets() {
        printf("offsets: %d %d %d\n", gyroOffsets[0], gyroOffsets[1], gyroOffsets[2]);
    }
};

extern TrackRayClass TrackRay;
