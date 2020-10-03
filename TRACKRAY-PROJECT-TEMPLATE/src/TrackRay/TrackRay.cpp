#include "TrackRay.h"
#include "Wire.h"
#include "Gyroscope.h"
#include "GyroCalibration.h"
#include "Preferences.h"

#include "DisplayCharactersSet.h"
#include "WiFiCaptain.h"
#include "cam.h"

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

        TrackRay.checkConnection();
        TrackRay.updateMotorsSpeed();
        TrackRay.displayText();
        camWebSocketLoop();

        vTaskDelay(20);
    }
}
void TR::updateGyro(void * param) {
    for(;;) {
        TrackRay.gyroUpdate();
        vTaskDelay(50);
    }
}

void trrBegin() {
    TrackRay.begin();
}

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

void trrControlMovement(const int8_t joystickX, const int8_t joystickY) {
    TrackRay.controlMovement(joystickX, joystickY);
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

void trrCanonShoot(const uint16_t durationMs) {
    TrackRay.canonShoot(durationMs);
}

void trrBuzzerBeep(const uint16_t durationMs) {
    TrackRay.buzzerBeep(durationMs);
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
void trrGyroUpdate() {
    TrackRay.gyroUpdate();
}

void trrDisplayDigit(const uint8_t digitID) {
    TrackRay.displayDigit(digitID);
}
void trrDisplayChar(const char letter) {
    TrackRay.displayChar(letter);
}
void trrDisplayText(String text, bool sweep) {
    TrackRay.displayText(text, sweep);
}
bool trrIsDisplayingText() {
    return TrackRay.isDisplayingText();
}

void trrWiFiControlStart(String wifiName, String wifiPassword) {
    TrackRay.startWiFiCaptain(wifiName, wifiPassword);
}
String trrCommandGet() {
    return TrackRay.commandGet();
}
String trrCommandGetIndexed(uint8_t index) {
    return TrackRay.commandGetIndexed(index);
}
void trrCommandClear() {
    TrackRay.commandClear();
}
void trrCommandSend(String command) {
    TrackRay.commandSend(command);
}
void trrCamEnable() {
    camStreamEnable();
}
void trrCamDisable() {
    camStreamDisable();
}

TrackRayClass::TrackRayClass(void) {
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
    if(connectionEnabled == true && connectionActive == false) {
        for(uint8_t i = 0; i < 3; ++i) {
            motorsSpeed[i] = 0;
        }
    }
    if(shootingEnd != 0 && millis() > shootingEnd) {
        motorsSpeed[2] = 0;
        shootingEnd = 0;
    }
    if(beepingEnd != 0 && millis() > beepingEnd) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT27]], 0);
        beepingEnd = 0;
    }

    // Enable H-bridge output 
    if(motorsSpeed[0] != 0 || motorsSpeed[1] != 0 || motorsSpeed[2]) {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT28]], 100);
    }
    else {
        TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT28]], 0);
    }

    // Filter motor speeds and turn off when connection not active
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

void TrackRayClass::controlMovement(const int8_t joystickX, const int8_t joystickY) {
    prevCommunicationTime = millis();
    int16_t engineLeftSpeed = 0;
    int16_t engineRightSpeed = 0;

    engineLeftSpeed = (joystickY + ((joystickY >= 0) ? 1 : -1) * joystickX);
    engineRightSpeed = (joystickY - ((joystickY >= 0) ? 1 : -1) * joystickX);

    engineLeftSpeed = constrain(engineLeftSpeed, -100, 100);
    engineRightSpeed = constrain(engineRightSpeed, -100, 100);

    setMotorsSpeed(engineLeftSpeed, 0);
    setMotorsSpeed(engineRightSpeed, 1);
}

void TrackRayClass::canonShoot(const uint16_t length) {
    setMotorsSpeed(100, 2);
    shootingEnd = millis() + length;
}

void TrackRayClass::buzzerBeep(const uint16_t length) {
    TR::setPWM(TR::serialPWM[TR::pwm_index[TR_OUT27]], 100);
    beepingEnd = millis() + length;
}

void TrackRayClass::begin() {
    beginCalled = true;
    Serial.begin(115200);
    
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
    camInit();
    xTaskCreate(TR::updatePWM, "updatePWM", 10000 , (void*) 0, 1, NULL);
    //xTaskCreate(TR::updateGyro, "updateGyro", 10000 , (void*) 0, 1, NULL);
    //xTaskCreatePinnedToCore(TR::updateGyro, "updateGyro", 10000 , (void*) 0, 1, NULL, 1);
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

void TrackRayClass::displayDigit(const uint8_t digit) {
    if(digit > 9) {
        return;
    }
    displayChar(digit + 48);
}

void TrackRayClass::displayChar(const char letter, int8_t sweepRight, int8_t sweepDown) {
    uint8_t letterID = letter;
    if(letterID >= 97 && letterID <= 122) {
        letterID = letterID - 32;   // change to upper case letters
    }
    if(letterID < 40 || letterID > 90) {
        return;     // out of defined letters
    }
    letterID = letterID - 40;   // move to beginning of character set array
    for(uint8_t i = 0; i < 5; ++i) {
        for(uint8_t j = 0; j < 5; ++j) {
            int16_t ledIndex = (i + sweepDown)*5 + j + sweepRight + 1;
            if((i + sweepDown) >= 0 && (i + sweepDown) < 5 && (j + sweepRight) >= 0 && (j + sweepRight) < 5) {
                trrSetLedDigital(ledIndex, characterSet[letterID][i][j]);
            }
        }
    }
}

void TrackRayClass::displayText(String text, bool sweep) {
    static uint8_t letterIndex;
    static uint32_t prevMoveTime = 0;
    static bool sweeping;
    if(text.length() > 0) {
        displayTextBuffer = text;
        letterIndex = 0;
        prevMoveTime = millis();
        sweeping = sweep;
    }
    if(prevMoveTime == 0) {
        return;
    }

    if(sweeping) {
        if((millis() > prevMoveTime + TR::lettersSweepTimeout) && letterIndex <= (displayTextBuffer.length()*5)) {
            prevMoveTime = millis();

            uint8_t letterID1 = displayTextBuffer[letterIndex / 5];
            uint8_t letterID2 = displayTextBuffer[letterIndex / 5 + 1];

            trrSetLedAllDigital(0);
            displayChar(letterID1, -(letterIndex % 5));
            displayChar(letterID2, (5 - (letterIndex % 5)));

            if(letterIndex / 5 >= displayTextBuffer.length()) {
                displayTextBuffer = "";
                prevMoveTime = 0;
            }
            ++letterIndex;
        }
    }
    else {
        if(millis() > prevMoveTime + TR::lettersSwapTimeout - TR::lettersBlankTimeout) {
            trrSetLedAllDigital(0);
        }
        if((millis() > prevMoveTime + TR::lettersSwapTimeout) && letterIndex <= displayTextBuffer.length()) {
            prevMoveTime = millis();

            trrSetLedAllDigital(0);
            displayChar(displayTextBuffer[letterIndex]);

            if(letterIndex >= displayTextBuffer.length()) {
                displayTextBuffer = "";
                prevMoveTime = 0;
            }
            ++letterIndex;
        }
    }
}

bool TrackRayClass::isDisplayingText() {
    return !displayTextBuffer.isEmpty();
}

void TrackRayClass::startWiFiCaptain(String ssid, String password) {
    if(!beginCalled) {
        begin();
    }
    setApCredentials(ssid, password);
    wifiCaptInit();
    connectionEnabled = true;
    camWebSocketStart();
}

void TrackRayClass::checkConnection() {
    if(!connectionEnabled) {
        return;
    }
    if(millis() > prevCommunicationTime + TR::communicationTimeout) {
        connectionActive = false;
    }
    else {
        connectionActive = true;
    }
}

String TrackRayClass::commandGet() {
    String command = String(commandGetCaptain());
    command.toLowerCase();
    return command;
}

String TrackRayClass::commandGetIndexed(uint8_t index) {
    char commandBuffer[64];
    sprintf(commandBuffer, commandGetCaptain());
    const char delimiter[2] = " ";
    char *token;
    token = strtok((char *)commandBuffer, delimiter);
    for(uint8_t i = 0; i < index; ++i) {
        token = strtok(NULL, delimiter);
    }
    String command = String(token);
    command.toLowerCase();
    return command;
}

void TrackRayClass::commandClear() {
    commandClearCaptain();
}

void TrackRayClass::commandSend(String command) {
    commandSendCaptain(command);
}

TrackRayClass TrackRay;