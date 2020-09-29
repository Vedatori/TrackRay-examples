#include "TrackRay/TrackRay.h"

const uint8_t CONTROL_PERIOD = 50;
uint32_t prevControlTime = 0;

const uint16_t BLINK_PERIOD = 200;
uint32_t prevBlinkTime = 0;

void setup() {
    trrBegin();
    trrWiFiControlStart("TrackRay", "12345678");    // password length minimally 8 characters
    trrDisplayText(ipToDisp());
}

void loop()
{
    if(millis() > prevControlTime + CONTROL_PERIOD) {
        prevControlTime = millis();
        
        trrGyroUpdate();

        if(trrCommandGet() == "shoot") {
            trrCanonShoot(1000);
            trrCommandClear();
        }
        else if(trrCommandGetIndexed(0) == "flash") {
            if(trrCommandGetIndexed(1) == "on") {
                trrSetFlashLightAnalog(100);
            }
            else if(trrCommandGetIndexed(1) == "half") {
                trrSetFlashLightAnalog(50);
            }
            else if(trrCommandGetIndexed(1) == "off") {
                trrSetFlashLightAnalog(0);
            }
            else {
                trrSetFlashLightAnalog(trrCommandGetIndexed(1).toInt());
            }
            trrCommandClear();
        }
        else if(trrCommandGetIndexed(0) == "cam") {
            if(trrCommandGetIndexed(1) == "on") {
                trrCamEnable();
            }
            else if(trrCommandGetIndexed(1) == "off") {
                trrCamDisable();
            }
            trrCommandClear();
        }
        else if(trrCommandGetIndexed(0) == "yaw") {
            trrCommandSend(String(trrGyroYaw()));
            trrCommandClear();
        }
        else if(trrCommandGetIndexed(0) == "beep") {
            trrBuzzerBeep(500);
            trrCommandClear();
        }

        //printf("%f %f %f\n", trrGyroYaw(), trrGyroPitch(), trrGyroRoll());
    }

    if(millis() > prevBlinkTime + BLINK_PERIOD && !trrIsDisplayingText()) {
        prevBlinkTime = millis();
        static uint8_t i = 1;
        static uint8_t prevI = 1;
        if(++i > 25)
            i = 1;
        trrSetLedDigital(i, 1);
        trrSetLedDigital(prevI, 0);
        prevI = i;
    }
}