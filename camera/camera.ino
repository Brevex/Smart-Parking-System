#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "esp_camera.h"
#include "credentials.h"
#include "camera_pins.h"

const int kIrSensorPin = 40;
const long kDetectionThreshold = 5000;

unsigned long objectDetectedTime = 0;
bool objectPresent = false;
bool imageHasBeenCaptured = false;

const char* kApiHost = "api.platerecognizer.com";
const String kApiEndpoint = "/v1/plate-reader/";

#define ADAFRUIT_IO_SERVER "io.adafruit.com"
#define ADAFRUIT_IO_SERVERPORT 1883

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, ADAFRUIT_IO_SERVER, ADAFRUIT_IO_SERVERPORT, ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY);
Adafruit_MQTT_Publish parkingStatusFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/vaga-status"); 
Adafruit_MQTT_Publish licensePlateFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/vaga-placa"); 

String lastPlate = "";

void MqttConnect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Failed to connect to MQTT. Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;

    if (retries == 0) {
      Serial.println("Could not connect to MQTT. Restarting...");
      ESP.restart();
    }
  }

  Serial.println("MQTT Connected!");
}

void SendToAdafruitIO(const char* status, const char* plate) {
  MqttConnect();

  Serial.printf("Sending to Adafruit IO -> Status: %s, Plate: %s\n", status, plate);

  if (!parkingStatusFeed.publish(status)) {
    Serial.println("Failed to publish parking status.");
  } else {
    Serial.println("Parking status published successfully!");
  }

  if (!licensePlateFeed.publish(plate)) {
    Serial.println("Failed to publish plate.");
  } else {
    Serial.println("Plate published successfully!");
  }
}

bool InitCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_XGA;
  config.jpeg_quality = 12; // 0-63, lower = higher quality
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed with error 0x%x\n", err);
    return false;
  }

  Serial.println("Camera initialized successfully.");
  return true;
}

void CaptureAndSendImage() {
  Serial.println("Clearing camera buffer to get a fresh frame...");
  camera_fb_t *fb_clear = esp_camera_fb_get();

  if (fb_clear) {
    esp_camera_fb_return(fb_clear);
  }

  Serial.println("Capturing current image...");
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Failed to capture image from framebuffer.");
    return;
  }

  Serial.printf("Image captured! Size: %zu bytes\n", fb->len);

  WiFiClientSecure clientSecure;
  clientSecure.setInsecure();

  Serial.printf("Connecting to host: %s\n", kApiHost);

  if (!clientSecure.connect(kApiHost, 443)) {
    Serial.println("Failed to connect to host!");
    esp_camera_fb_return(fb);
    return;
  }

  Serial.println("Connection successful.");

  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String head = "--" + boundary + "\r\n" +
                "Content-Disposition: form-data; name=\"upload\"; filename=\"image.jpg\"\r\n" +
                "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t contentLength = head.length() + fb->len + tail.length();

  clientSecure.println("POST " + kApiEndpoint + " HTTP/1.1");
  clientSecure.println("Host: " + String(kApiHost));
  clientSecure.println("Authorization: Token " + String(apiToken));
  clientSecure.println("Content-Length: " + String(contentLength));
  clientSecure.println("Content-Type: multipart/form-data; boundary=" + boundary);
  clientSecure.println();
  clientSecure.print(head);

  uint8_t *buf = fb->buf;
  size_t len = fb->len;
  size_t sent = 0;

  while (len > 0) {
    size_t to_send = (len > 1024) ? 1024 : len;
    clientSecure.write(buf + sent, to_send);
    sent += to_send;
    len -= to_send;
  }

  clientSecure.print(tail);
  esp_camera_fb_return(fb);

  Serial.println("Image sent. Waiting for response...");

  long startTime = millis();

  while (clientSecure.connected() && !clientSecure.available() && millis() - startTime < 10000) {
    delay(100);
  }

  String responseBody = "";
  bool headersEnded = false;

  while (clientSecure.available()) {
    String line = clientSecure.readStringUntil('\n');

    if (!headersEnded) {
      if (line == "\r") {
        headersEnded = true;
      }
    } else {
      responseBody += line;
    }
  }

  clientSecure.stop();

  if (responseBody.length() > 0) {
    Serial.println("API response received:");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, responseBody);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    JsonArray results = doc["results"];

    if (!results.isNull() && results.size() > 0) {
      const char* plate = results[0]["plate"];

      if (plate) {
        Serial.println("======================================");
        Serial.print("PLATE RECOGNIZED: ");
        Serial.println(plate);
        Serial.println("======================================");

        lastPlate = String(plate);
        SendToAdafruitIO("Occupied", plate);

      } else {
        Serial.println("No plate found in the first result.");
        SendToAdafruitIO("Occupied", "N/A");
        lastPlate = "N/A";
      }
    } else {
      Serial.println("No plate results returned by API.");
      SendToAdafruitIO("Occupied", "API Error");
      lastPlate = "API Error";
    }
  } else {
    Serial.println("No response received from server or timeout.");
    SendToAdafruitIO("Occupied", "Timeout");
    lastPlate = "Timeout";
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nStarting parking monitoring system...");

  pinMode(kIrSensorPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!InitCamera()) {
    Serial.println("FATAL ERROR: Failed to initialize camera. Restarting in 10s...");
    delay(10000);
    ESP.restart();
  }

  SendToAdafruitIO("Available", "");

  Serial.println("\nSystem ready. Waiting for vehicle...");
}

void loop() {
  MqttConnect();
  mqtt.processPackets(100);

  objectPresent = (digitalRead(kIrSensorPin) == LOW);

  if (objectPresent && objectDetectedTime == 0) {
    Serial.println("Vehicle detected. Starting countdown...");
    objectDetectedTime = millis();
    imageHasBeenCaptured = false;
  }

  if (objectPresent && objectDetectedTime > 0 && !imageHasBeenCaptured) {
    if (millis() - objectDetectedTime > kDetectionThreshold) {
      Serial.println("Vehicle stable. Initiating capture.");

      CaptureAndSendImage();

      imageHasBeenCaptured = true;

      Serial.println("\nCapture complete. Waiting for vehicle to leave to rearm the system.");
    }
  }

  if (!objectPresent) {
    if (objectDetectedTime > 0) {
      Serial.println("Vehicle left. System rearmed and ready for next detection.");
      SendToAdafruitIO("Available", "");
      lastPlate = "";
    }
    objectDetectedTime = 0;
  }

  delay(100);
}
