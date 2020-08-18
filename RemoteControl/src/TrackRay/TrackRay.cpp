#include "TrackRay.h"
#include "Wire.h"
#include "Gyroscope.h"
#include "GyroCalibration.h"
#include "Preferences.h"

const uint8_t TR::PWM_MAX = rb::SerialPWM::resolution();
rb::SerialPWM TR::serialPWM(TR::PWM_CHANNELS, { TR::REG_DAT }, TR::REG_LATCH, TR::REG_CLK, -1, TR::PWM_FREQUENCY);
int8_t TR::pwm_index[] = {0, 3, 2, 29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18, 13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0};

void TR::setPWM(rb::SerialPWM::value_type& channel, int8_t power) {
    if(power > TR::PWM_MAX)
        power = TR::PWM_MAX;
    else if(power < 0)
        power = 0;

    channel = power;
}

void TR::updatePWM(void * param) {
    for(;;) {
        TR::serialPWM.update();
        ledcDetachPin(TR::LIGHT_PIN);
        pinMode(TR::LIGHT_PIN, INPUT);
        TrackRay.setButton(digitalRead(TR::LIGHT_PIN));
        pinMode(TR::LIGHT_PIN, OUTPUT);
        ledcAttachPin(TR::LIGHT_PIN, TR::LIGHT_PWM_CHANNEL);
        digitalWrite(TR::LIGHT_PIN, 0);
        TrackRay.updateMotorsSpeed();
        TrackRay.gyroUpdate();

        vTaskDelay(20);
    }
}
bool digit[10][5][5] = {
    {   // 0
        {0,1,1,1,0},
        {0,1,0,1,0},
        {0,1,0,1,0},
        {0,1,0,1,0},
        {0,1,1,1,0}
    }, {// 1
        {0,0,0,1,0},
        {0,0,1,1,0},
        {0,1,0,1,0},
        {0,0,0,1,0},
        {0,0,0,1,0}
    }, {// 2
        {0,0,1,0,0},
        {0,1,0,1,0},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,1,1,1,0}
    }, {// 3
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,0,0,1,0},
        {0,1,1,0,0}
    }, {// 4
        {0,1,0,0,0},
        {0,1,0,0,0},
        {0,1,1,1,0},
        {0,0,1,0,0},
        {0,0,1,0,0}
    }, {// 5
        {0,1,1,1,0},
        {0,1,0,0,0},
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,1,1,1,0}
    }, {// 6
        {0,1,1,1,0},
        {0,1,0,0,0},
        {0,1,1,1,0},
        {0,1,0,1,0},
        {0,1,1,1,0}
    }, {// 7
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,1,0,0,0},
        {0,1,0,0,0}
    }, {// 8
        {0,1,1,1,0},
        {0,1,0,1,0},
        {0,1,1,1,0},
        {0,1,0,1,0},
        {0,1,1,1,0}
    }, {// 9
        {0,1,1,1,0},
        {0,1,0,1,0},
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,1,1,1,0}
    },
};


bool trrReadButton() {
    return TrackRay.getButton(); 
}

void trrSetLedDigital(int8_t pin, const bool state) {
    if(pin >= 1 && pin <= 25)
        TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], state * TR::PWM_MAX);
        
}
void trrSetLedAnalog(int8_t pin, const int8_t value) {
    if(pin >= 1 && pin <= 25)
        TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], value);
}
void trrSetLedAllDigital(const bool state) {
    for(uint8_t i = 1; i <= 25; ++i) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[i]], state * TR::PWM_MAX);
    }
}
void trrSetLedAllAnalog(const int8_t value) {
    for(uint8_t i = 1; i <= 25; ++i) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[i]], value);
    }
}
void trrSetFlashLightDigital(const bool state) {
    TrackRay.setFlashLight(state * 255);
}
void trrSetFlashLightAnalog(const int8_t value) {
    TrackRay.setFlashLight(int(value * 2.5));
}

void trrMotorsSetSpeed(const int8_t speedLeft, const int8_t speedRight) {
    TrackRay.setMotorsSpeed(speedLeft, 0);
    TrackRay.setMotorsSpeed(speedRight, 1);
}
void trrMotorsSetSpeedLeft(const int8_t speed) {
    TrackRay.setMotorsSpeed(speed, 0);
}
void trrMotorsSetSpeedRight(const int8_t speed) {
    TrackRay.setMotorsSpeed(speed, 1);
}
void trrCanonSetSpeed(const int8_t speed) {
    TrackRay.setMotorsSpeed(speed, 2);
}

bool trrGetGyroEnabled() {
    return TrackRay.gyroGetEnabled();
}
float trrGyroYaw() {
    return TrackRay.gyroData(0);
}
float trrGyroPitch() {
    return TrackRay.gyroData(1);
}
float trrGyroRoll() {
    return TrackRay.gyroData(2);
}
void trrGyroData(float ypr[]) {
    for(uint8_t i = 0; i < 3; ++i) {
        ypr[i] = TrackRay.gyroData(i);
    }
}
void trrGyroCalibrate() {
    TrackRay.gyroCalibrate();
    trrSetLedAllDigital(1);
}

void trrDisplayDigit(const uint8_t digitID) {
    TrackRay.displayDigit(digitID);
}
void trrDisplayChar(const char letter) {
    TrackRay.displayChar(letter);
}
void trrDisplayText(String text, uint8_t repetitions) {
    TrackRay.displayText(text, repetitions);
}

TrackRayClass::TrackRayClass(void) {
    Serial.begin(115200);
    ledcSetup(TR::LIGHT_PWM_CHANNEL, TR::LIGHT_PWM_FREQUENCY, TR::LIGHT_PWM_RESOLUTION);
    ledcAttachPin(TR::LIGHT_PIN, TR::LIGHT_PWM_CHANNEL);
    for(uint8_t i = 0; i < 3; ++i) {
        motorsSpeed[i] = 0;
        motorsSpeedFiltered[i] = 0;
        gyroYPR[i] = 0;
        gyroOffsets[i] = 0;
    }
}

bool TrackRayClass::getButton() {
    return buttonPressed;
}
void TrackRayClass::setButton(bool pressed) {
    TrackRay.buttonPressed = pressed;
}
void TrackRayClass::setFlashLight(int16_t brightness) {
    if(brightness < 0)
        lightBrightness = 0;
    else if(brightness > 255)
        lightBrightness = 255;
    else
        lightBrightness = brightness;
    ledcWrite(TR::LIGHT_PWM_CHANNEL, lightBrightness);
}
void TrackRayClass::setMotorsSpeed(const int8_t speed, const int8_t index) {
    if(index == 0 || index == 1) {
        if(speed < -100)
            motorsSpeed[index] = -100;
        else if(speed > 100)
            motorsSpeed[index] = 100;
        else
            motorsSpeed[index] = speed;
    }
    else if(index == 2) {
        if(speed < 0)
            motorsSpeed[index] = 0;
        else if(speed > 100)
            motorsSpeed[index] = 100;
        else
            motorsSpeed[index] = speed;
    }
    
}
void TrackRayClass::updateMotorsSpeed() {
    // Enable H-bridge output 
    if(motorsSpeed[0] != 0 || motorsSpeed[1] != 0) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT28]], 100);
    }
    else {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT28]], 0);
    }

    // Filter motor speeds
    for(uint8_t i = 0; i < 3; ++i) {
        motorsSpeedFiltered[i] = motorsSpeed[i] * TR::MOTOR_SPEED_FILTER_UPDATE_COEF + motorsSpeedFiltered[i] * (1 - TR::MOTOR_SPEED_FILTER_UPDATE_COEF);
    }

    // Left motor
    if(motorsSpeed[0] > 0) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT31]], 0);
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT32]], (int)motorsSpeedFiltered[0]);
    }
    else {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT31]], -(int)motorsSpeedFiltered[0]);
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT32]], 0);
    }

    // Right motor
    if(motorsSpeed[1] > 0) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT29]], 0);
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT30]], (int)motorsSpeedFiltered[1]);
    }
    else {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT29]], -(int)motorsSpeedFiltered[1]);
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT30]], 0);
    }

    // Cannon motor
    TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT26]], (int)motorsSpeedFiltered[2]);
}

void TrackRayClass::begin() {
    preferences.begin("trackray", false);
    gyroOffsets[0] = preferences.getInt("counter", 0);
    gyroOffsets[1] = preferences.getInt("gyroOffPitch", 0);
    gyroOffsets[2] = preferences.getInt("gyroOffRoll", 0);
    bool prefsPresent = preferences.getBool("prefsPresent", false);
    preferences.end();

    Wire.begin(15, 13);
    if(initiateGyroscope(gyroOffsets)) {
        gyroEnabled = true;
        
        // Calibrate gyroscope if no offset data present
        if(prefsPresent == false) {
            printf("Calibrating gyroscope: Place TrackRay electronics board horizontally and wait.\n");
            trrGyroCalibrate();
        }
    }
    xTaskCreate(TR::updatePWM, "updatePWM", 10000 , (void*) 0, 1, NULL);
}

bool TrackRayClass::gyroGetEnabled() {
    return gyroEnabled;
}
float TrackRayClass::gyroData(uint8_t index) {
    float output = gyroYPR[index];
    if(index == 1) {
        output -= 20;
        if(output < -180) {
            output += 360;
        }
        output = -output;
    }
    return output;
}
void TrackRayClass::gyroCalibrate() {
    gyroCalibration(gyroOffsets);

    preferences.begin("trackray", false);
    preferences.putInt("counter", gyroOffsets[0]);
    preferences.putInt("gyroOffPitch", gyroOffsets[1]);
    preferences.putInt("gyroOffRoll", gyroOffsets[2]);
    preferences.putBool("prefsPresent", true);
    preferences.end();
}
void TrackRayClass::gyroUpdate() {
    updateGyroData(gyroYPR);
}

void TrackRayClass::displayDigit(const uint8_t digitID) {
    for(uint8_t i = 0; i < 5; ++i) {
        for(uint8_t j = 0; j < 5; ++j) {
            trrSetLedDigital( i*5 + j + 1, digit[digitID][i][j]);
        }
    }
}
void TrackRayClass::displayChar(const char letter) {
    if(letter < 48 || letter > 57) {
        return;
    }
    displayDigit(letter - 48);
}
void TrackRayClass::displayText(String text, uint8_t repetitions) {
    for(uint8_t i = 0; i < repetitions; ++i) {
        trrSetLedAllDigital(1);
        delay(200);
        for(uint8_t i = 0; i < text.length(); ++i) {
            displayChar(text[i]);
            delay(500);
            trrSetLedAllDigital(0);
            delay(50);
        }
        trrSetLedAllDigital(1);
        delay(500);
        trrSetLedAllDigital(0);
    }
}

TrackRayClass TrackRay;