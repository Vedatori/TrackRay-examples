#include "TrackRay.h"
#include "Wire.h"
#include "Gyroscope.h"
#include "GyroCalibration.h"
#include "Preferences.h"

const uint8_t TR::PWM_MAX = rb::SerialPWM::resolution();
rb::SerialPWM TR::serialPWM(TR::PWM_CHANNELS, { TR::REG_DAT }, TR::REG_LATCH, TR::REG_CLK, -1, TR::PWM_FREQUENCY);
int8_t TR::pwm_index[] = { 3, 2, 29, 28, 31, 30, 25, 24, 27, 26, 21, 20, 23, 22, 17, 16, 19, 18, 13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0};

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
        vTaskDelay(20);
    }
}

bool trrReadButton() {
    return TrackRay.getButton(); 
}

void trrSetLedDigital(int8_t pin, const bool state) {
    if(pin >= TR_OUT1 && pin <= TR_OUT25)
        TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], state * TR::PWM_MAX);
        
}
void trrSetLedAnalog(int8_t pin, const int8_t value) {
    if(pin >= TR_OUT1 && pin <= TR_OUT25)
        TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], value);
}
void trrSetLedAllDigital(const bool state) {
    for(uint8_t i = 0; i < 25; ++i) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[i]], state * TR::PWM_MAX);
    }
}
void trrSetLedAllAnalog(const int8_t value) {
    for(uint8_t i = 0; i < 25; ++i) {
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
}

TrackRayClass::TrackRayClass(void) {
    ledcSetup(TR::LIGHT_PWM_CHANNEL, TR::LIGHT_PWM_FREQUENCY, TR::LIGHT_PWM_RESOLUTION);
    ledcAttachPin(TR::LIGHT_PIN, TR::LIGHT_PWM_CHANNEL);
    xTaskCreate(TR::updatePWM, "updatePWM", configMINIMAL_STACK_SIZE , (void*) 0, 1, NULL);
    for(uint8_t i = 0; i < 3; ++i) {
        motorsSpeed[i] = 0;
        motorsSpeedFiltered[i] = 0;
        gyroYPR[i] = 0;
        gyroOffsets[i] = 0;
    }
    
    Serial.begin(115200);
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

TrackRayClass TrackRay;