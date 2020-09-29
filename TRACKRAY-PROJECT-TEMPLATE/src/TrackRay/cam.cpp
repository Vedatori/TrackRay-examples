#include "cam.h"

bool clientConnected[3] = {false, false, false};
uint8_t clientNum = 0;
WebSocketsServer webSocketCam = WebSocketsServer(82);
camera_fb_t * fb = NULL;
uint32_t lastImage = 0;
uint32_t periodImage = 330;
bool camInitSuccess = false;
bool streamEnabled = false;
int8_t minCamSignalDbm = -70;

void camInit() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    //init with high specs to pre-allocate larger buffers
    config.frame_size = FRAMESIZE_HQVGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
     
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        camInitSuccess = false;
        return;
    }
    camInitSuccess = true;
    
    sensor_t * s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_HQVGA);
}

void camSendImage() {
    if(!camInitSuccess) {
        return;
    }
    if(WiFi.RSSI() > minCamSignalDbm && (clientConnected[clientNum] && streamEnabled)) {
        fb = esp_camera_fb_get();
        webSocketCam.sendBIN(clientNum, (const uint8_t *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        fb = NULL;
        //printf("image sent\n");
    }
}

void camWebSocketStart() {
    if(!camInitSuccess) {
        return;
    }
    webSocketCam.begin();
    webSocketCam.onEvent(onWebSocketEventCam);
}

void camWebSocketLoop() {
    if(!camInitSuccess) {
        return;
    }
    webSocketCam.loop();
    if(millis() > lastImage + periodImage) {
        lastImage = millis();
        camSendImage();
    }
}

void camStreamEnable() {
    streamEnabled = true;
}

void camStreamDisable() {
    streamEnabled = false;
}

void onWebSocketEventCam(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {
    clientNum = client_num;
    switch(type) {
        case WStype_DISCONNECTED:
            if(client_num < 3) {
                clientConnected[client_num] = false;
                streamEnabled = false;
            }
            break;
        case WStype_CONNECTED:
            if(client_num < 3) {
                clientConnected[client_num] = true;
            }
            break;
        default:
            break;
  }
}