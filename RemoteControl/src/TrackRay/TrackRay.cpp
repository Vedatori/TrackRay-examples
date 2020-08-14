#include "TrackRay.h"

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
        vTaskDelay(20);
    }
}

void digitalWrite(shiftRegPins pin, const bool state) {
    TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], state * TR::PWM_MAX );
}
bool digitalRead() {
    return TrackRay.getButton(); 
}
void analogWrite(shiftRegPins pin, const int8_t value) {
    if(pin != TR_OUT_LIGHT)
        TR::setPWM(TR::serialPWM[TR::pwm_index[pin]], value );
    else
        TrackRay.setFlashLight(value);
    
}

TrackRayClass::TrackRayClass(void) {
    ledcSetup(TR::LIGHT_PWM_CHANNEL, TR::LIGHT_PWM_FREQUENCY, TR::LIGHT_PWM_RESOLUTION);
    ledcAttachPin(TR::LIGHT_PIN, TR::LIGHT_PWM_CHANNEL);
    xTaskCreate(TR::updatePWM, "updatePWM", configMINIMAL_STACK_SIZE , (void*) 0, 1, NULL);
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

void TrackRayClass::begin() {
}


TrackRayClass TrackRay;