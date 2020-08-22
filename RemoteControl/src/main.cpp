#include <Arduino.h>
#include <DNSserver.h>            //Local DNS webserver used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Webserver.h>
#include <WebSocketsServer.h>   //https://github.com/Links2004/arduinoWebSockets
#include <Wire.h>

#include "TrackRay/TrackRay.h"
#include "credentials.h"

WebSocketsServer webSocket = WebSocketsServer(1337);
WebServer webserver(80);
WiFiManager wifiManager;

const uint8_t CONTROL_PERIOD = 50;
uint32_t prevControlTime = 0;

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
            const char delimiter[2] = ",";
            char *token;
            token = strtok((char *)payload, delimiter);
            char controlMsg[] = "control";
            char commandMsg[] = "command";
            int8_t joystickX = 0, joystickY = 0;
            if(strcmp(token, controlMsg) == 0) {
                token = strtok(NULL, delimiter);
                joystickX = atoi(token);
                token = strtok(NULL, delimiter);
                joystickY = atoi(token);
                trrControlMovement(joystickX, joystickY);
            }
            else if(strcmp(token, commandMsg) == 0) {
                token = strtok(NULL, delimiter);
                if(strcmp(token, "shoot") == 0) {
                    trrCanonShoot(1000);
                }
                else if(strcmp(token, "flash on") == 0) {
                    trrSetFlashLightAnalog(100);
                }
                else if(strcmp(token, "flash half") == 0) {
                    trrSetFlashLightAnalog(50);
                }
                else if(strcmp(token, "flash off") == 0) {
                    trrSetFlashLightAnalog(0);
                }

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
        trrGyroUpdate();

        printf("%d %f %f %f\n", trrReadButton(), trrGyroYaw(), trrGyroPitch(), trrGyroRoll());
    }

    if(millis() > prevBlinkTime + BLINK_PERIOD) {
        prevBlinkTime = millis();
        static uint8_t i = 0;
        static uint8_t prevI = 0;
        if(i++ > 25)
            i = 0;
        trrSetLedDigital(i, 1);
        trrSetLedDigital(prevI, 0);
        prevI = i;
    }
}