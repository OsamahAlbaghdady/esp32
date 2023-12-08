#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <DHT.h>  // Include the DHT library
#define CAMERA_MODEL_AI_THINKER
#include <stdio.h>
#include "camera_pins.h"

const char* ssid = "esp32";
const char* password = "12345678";
const char* websocket_server_host = "192.168.1.100";
const uint16_t websocket_server_port1 = 8888;
int flashlight = 0;
int relay1 = 0;
#define RESET_BUTTON_PIN 4
#define DHTPIN 14      // Change this to the GPIO pin you connected the DHT22 to
#define DHTTYPE DHT22  // Change this to DHT11 if you are using that sensor

DHT dht(DHTPIN, DHTTYPE);

using namespace websockets;
WebsocketsClient client;

void onEventsCallback(WebsocketsEvent event, String data) {
    Serial.println("soket Opened");

    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connection Closed");
        ESP.restart();
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}



void setup() 
{
  Serial.println("kassim");
  camera_config_t config;
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
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
  
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 40;
  config.fb_count = 2;

  dht.begin();

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) { return; }

  sensor_t * s = esp_camera_sensor_get();

  s->set_contrast(s, 0);   
  s->set_raw_gma(s, 1);
  s->set_vflip(s, 1);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { delay(500); }


  client.onEvent(onEventsCallback);

  Serial.begin(115200);
  while(!client.connect(websocket_server_host, websocket_server_port1, "/")) { delay(500); }
}

void loop() 
{
if (digitalRead(RESET_BUTTON_PIN) != LOW) {
        Serial.println("Reset button pressed. Resetting...");
        ESP.restart();  // Reset the ESP32
    }
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    Serial.println(temperature);
    Serial.println(humidity);
    Serial.println("osama");
  client.poll();
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb)
  {
    esp_camera_fb_return(fb);
    return;
  }


  client.sendBinary((const char*) fb->buf, fb->len);

  
  esp_camera_fb_return(fb);


  String temp = "temperature=" + String(temperature) + ",humidity=" + String(humidity); 
  
  client.send(temp);

}
