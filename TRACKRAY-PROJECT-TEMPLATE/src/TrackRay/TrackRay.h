#pragma once

#include "RBControl_serialPWM.hpp"
#include <Arduino.h>
#include "Preferences.h"

#include "WiFiCaptain.h"
#include "cam.h"

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
const uint8_t LIGHT_PWM_CHANNEL = 2;
const uint16_t LIGHT_PWM_FREQUENCY = 5000;
const uint8_t LIGHT_PWM_RESOLUTION = 8;
const float MOTOR_SPEED_FILTER_UPDATE_COEF = 0.15;
const char STORAGE_NAMESPACE[] = "trackray";
const uint16_t communicationTimeout = 1000;
const uint16_t shootingLength = 3000;
const uint16_t lettersSwapTimeout = 500;
const uint16_t lettersBlankTimeout = 50;
const uint16_t lettersSweepTimeout =  150;

extern rb::SerialPWM serialPWM;
extern int8_t pwm_index[33];

void setPWM(rb::SerialPWM::value_type& channel, int8_t power);
void updatePWM(void * param);
void updateGyro(void * param);
}

enum shiftRegPins {
    D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, 
    D17, D18, D19, D20, D21, D22, D23, D24, D25, 
    TR_OUT26, TR_OUT27, TR_OUT28, TR_OUT29, TR_OUT30, TR_OUT31, TR_OUT32,
};

void trrBegin();

bool trrReadButton();

void trrSetLedDigital(int8_t pin, const bool state);
void trrSetLedAnalog(int8_t pin, const int8_t value);
void trrSetLedAllDigital(const bool state);
void trrSetLedAllAnalog(const int8_t value);
void trrSetFlashLightDigital(const bool state);
void trrSetFlashLightAnalog(const int8_t value);

void trrControlMovement(const int8_t joystickX, const int8_t joystickY);
void trrMotorsSetSpeed(const int8_t speedLeft, const int8_t speedRight);
void trrMotorsSetSpeedLeft(const int8_t speed);
void trrMotorsSetSpeedRight(const int8_t speed);
void trrCanonShoot(const uint16_t durationMs = 1000);
void trrBuzzerBeep(const uint16_t durationMs = 500);

bool trrGyroEnabled();
float trrGyroYaw();
float trrGyroPitch();
float trrGyroRoll();
void trrGyroData(float ypr[]);
void trrGyroCalibrate();
void trrGyroUpdate();

void trrDisplayDigit(const uint8_t digitID);
void trrDisplayChar(const char letter);
void trrDisplayText(String text, bool sweep = true);
bool trrIsDisplayingText();

void trrWiFiControlStart(String wifiName, String wifiPassword);
String trrCommandGet();
String trrCommandGetIndexed(uint8_t index);
void trrCommandClear();
void trrCommandSend(String command);
void trrCamEnable();
void trrCamDisable();

class TrackRayClass {
    bool beginCalled = false;
    bool buttonPressed = false;
    uint8_t lightBrightness = 0;
    int8_t motorsSpeed[3];
    float motorsSpeedFiltered[3];
    bool gyroEnabled = false;
    float gyroYPR[3];
    int16_t gyroOffsets[3];
    Preferences preferences;
    uint32_t prevCommunicationTime = 0;
    bool connectionEnabled = false;
    bool connectionActive = false;
    uint32_t shootingEnd = 0;
    uint32_t beepingEnd = 0;
    String displayTextBuffer;

public:
    TrackRayClass();
    bool getButton();
    void setButton(bool pressed);
    void setFlashLight(int16_t brightness);
    void setMotorsSpeed(const int8_t speed, const int8_t index);
    void updateMotorsSpeed();
    void controlMovement(const int8_t joystickX, const int8_t joystickY);
    void canonShoot(const uint16_t length);
    void buzzerBeep(const uint16_t length);
    void begin();
    
    bool gyroGetEnabled();
    float gyroData(uint8_t index) ;
    void gyroCalibrate();
    void gyroUpdate();
    void printOffsets() {
        printf("offsets: %d %d %d\n", gyroOffsets[0], gyroOffsets[1], gyroOffsets[2]);
    }

    void displayDigit(const uint8_t digit);
    void displayChar(const char letter, int8_t sweepRight = 0, int8_t sweepDown = 0);
    void displayText(String text = "", bool sweep = true);
    bool isDisplayingText();

    void startWiFiCaptain(String ssid, String password);
    void checkConnection();
    String commandGet();
    String commandGetIndexed(uint8_t index);
    void commandClear();
    void commandSend(String command);
};

extern TrackRayClass TrackRay;
