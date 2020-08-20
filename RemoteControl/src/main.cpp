#include <Arduino.h>
#include <DNSserver.h>            //Local DNS webserver used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Webserver.h>
#include <WebSocketsServer.h>   //https://github.com/Links2004/arduinoWebSockets
#include <Wire.h>

#include "StateVector.h"
#include "TrackRay/TrackRay.h"
#include "credentials.h"

WebSocketsServer webSocket = WebSocketsServer(1337);
WebServer webserver(80);
WiFiManager wifiManager;

StateVector stateVector;

const uint8_t CONTROL_PERIOD = 50;
uint32_t prevControlTime = 0;
const uint16_t communicationTimeout = 1000;
uint32_t prevCommunicationTime = 0;

const uint16_t BLINK_PERIOD = 200;
uint32_t prevBlinkTime = 0;

// Callback: receiving any WebSocket message
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {

    // Figure out the type of WebSocket event
    switch(type) {

        // Client has disconnected
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", client_num);
            break;

        // New client has connected
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(client_num);
            Serial.printf("[%u] Connection from ", client_num);
            Serial.println(ip.toString());
        }
        break;

        // Handle text messages from client
        case WStype_TEXT:{
            //printf("[%u] Received text: %s\n", client_num, payload);
            const char delimiter[2] = ",";
            char *token;
            token = strtok((char *)payload, delimiter);
            char controlMsg[] = "control";
            if(strcmp(token, controlMsg) == 0) {
                token = strtok(NULL, delimiter);
                stateVector.joystickX = atoi(token);
                token = strtok(NULL, delimiter);
                stateVector.joystickY = atoi(token);
                //printf("%d %d", stateVector.joystickX, stateVector.joystickY);
                prevCommunicationTime = millis();
            }
        }



            //webSocket.sendTXT(client_num, msg_buf);
            break;

        // For everything else: do nothing
        /*case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:*/
        default:
            break;
    }
}

void handleClients(void * param) {
    for(;;) {
        webserver.handleClient();
        vTaskDelay(200);
    }
}

void setup() {
    TrackRay.begin();

    wifiManager.autoConnect("TrackRay");
    //WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());   // You can get IP address assigned to ESP
    trrDisplayText(WiFi.localIP().toString(), 1);
    
    //initialize SPIFFS to be able to serve up the static HTML files. 
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS Mount failed");
    } 
    else {
        Serial.println("SPIFFS Mount succesfull");
    }

    webserver.serveStatic("/", SPIFFS, "/webApp.html");
    webserver.serveStatic("/style.css", SPIFFS, "/style.css");
    
    webserver.begin();

    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);

    //xTaskCreate(handleClients, "handleClients", 1024 * 4 , (void*) 0, 1, NULL);
}

void loop()
{
    vTaskDelay(1);
    webserver.handleClient();

    if(millis() > prevControlTime + CONTROL_PERIOD) {
        prevControlTime = millis();
        
        webSocket.loop();

        if(millis() > prevCommunicationTime + communicationTimeout) {
            stateVector.joystickX = 0;
            stateVector.joystickY = 0;
        }
        stateVector.engineLeftSpeed = (stateVector.joystickY + ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX);
        stateVector.engineRightSpeed = (stateVector.joystickY - ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX);

        stateVector.engineLeftSpeed = constrain(stateVector.engineLeftSpeed, -100, 100);
        stateVector.engineRightSpeed = constrain(stateVector.engineRightSpeed, -100, 100);

        trrMotorsSetSpeed(stateVector.engineLeftSpeed, stateVector.engineRightSpeed);

        printf("%d %d %d %f %f %f\n", stateVector.engineLeftSpeed, stateVector.engineRightSpeed, trrReadButton(), trrGyroYaw(), trrGyroPitch(), trrGyroRoll());
    }

    if(millis() > prevBlinkTime + BLINK_PERIOD) {
        prevBlinkTime = millis();
        static bool ledOn = 0;
        //digitalWrite(TR1, ledOn);
        ledOn = !ledOn;
        static uint8_t i = 0;
        static uint8_t prevI = 0;
        if(i++ > 25)
            i = 0;
        trrSetLedDigital(i, 1);
        trrSetLedDigital(prevI, 0);
        prevI = i;
    }
}