#include <Arduino.h>
#include <DNSserver.h>            //Local DNS webserver used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Webserver.h>
#include <ArduinoWebsockets.h>
#include "esp_camera.h"
#include <Wire.h>

#include "StateVector.h"
#include "TrackRay/TrackRay.h"
#include "credentials.h"

websockets::WebsocketsServer server;
WebServer webserver(80);
WiFiManager wifiManager;

void handleClients(void * param) {
    for(;;) {
        webserver.handleClient();
        vTaskDelay(200);
    }
}

StateVector stateVector;

const uint8_t CONTROL_PERIOD = 50;
uint32_t prevControlTime = 0;
const uint16_t communicationTimeout = 1000;
uint32_t prevCommunicationTime = 0;

const uint16_t BLINK_PERIOD = 50;
uint32_t prevBlinkTime = 0;

//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
void handleJSData() {
    stateVector.joystickX = webserver.arg(0).toInt();
    stateVector.joystickY = webserver.arg(1).toInt();

    //return an HTTP 200
    webserver.send(200, "text/plain", "");
    prevCommunicationTime = millis();
}

void setup() {
    TrackRay.begin();
    Serial.begin(115200);

    wifiManager.autoConnect("TrackRay");
    //WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());   // You can get IP address assigned to ESP
    
    //initialize SPIFFS to be able to serve up the static HTML files. 
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS Mount failed");
    } 
    else {
        Serial.println("SPIFFS Mount succesfull");
    }
    //set the static pages on SPIFFS for the html and js
    webserver.serveStatic("/", SPIFFS, "/joystick.html"); 
    //call handleJSData function when this URL is accessed by the js in the html file
    webserver.on("/jsData.html", handleJSData);
    webserver.begin();
    server.listen(82);

    //xTaskCreate(handleClients, "handleClients", 1024 * 4 , (void*) 0, 1, NULL);
}

void loop()
{
    vTaskDelay(1);
    webserver.handleClient();
    if(millis() > prevControlTime + CONTROL_PERIOD) {
        prevControlTime = millis();

        if(millis() > prevCommunicationTime + communicationTimeout) {
            stateVector.joystickX = 0;
            stateVector.joystickY = 0;
        }
        stateVector.engineLeftSpeed = (stateVector.joystickY + ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX);
        stateVector.engineRightSpeed = (stateVector.joystickY - ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX);

        stateVector.engineLeftSpeed = constrain(stateVector.engineLeftSpeed, -100, 100);
        stateVector.engineRightSpeed = constrain(stateVector.engineRightSpeed, -100, 100);

        static bool trackSide = 0;

        /*if(trackSide) 
            stateVector.engineLeftSpeed = 0;
        else
            stateVector.engineRightSpeed = 0;*/

        trackSide = !trackSide;

        analogWrite(TR_OUT28, 100);
        if(stateVector.engineRightSpeed > 0) {
            analogWrite(TR_OUT29, 0);
            analogWrite(TR_OUT30, stateVector.engineRightSpeed);
        }
        else {
            analogWrite(TR_OUT29, -stateVector.engineRightSpeed);
            analogWrite(TR_OUT30, 0);
        }

        if(stateVector.engineLeftSpeed > 0) {
            analogWrite(TR_OUT31, 0);
            analogWrite(TR_OUT32, stateVector.engineLeftSpeed);
        }
        else {
            analogWrite(TR_OUT31, -stateVector.engineLeftSpeed);
            analogWrite(TR_OUT32, 0);
        }

        Serial.printf("%d %d %d\n", stateVector.engineLeftSpeed, stateVector.engineRightSpeed, digitalRead());
    }

    if(millis() > prevBlinkTime + BLINK_PERIOD) {
        prevBlinkTime = millis();
        static bool ledOn = 0;
        //digitalWrite(TR1, ledOn);
        ledOn = !ledOn;
        static uint8_t i = 0;
        static uint8_t prevI = 0;
        if(i++ > 24)
            i = 0;
        digitalWrite((shiftRegPins) i, 1);
        digitalWrite((shiftRegPins) prevI, 0);
        analogWrite(TR_OUT_LIGHT, i*10);
        prevI = i;
    }
}