#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

const char* ssid = "";
const char* password = "";
const char* apiToken = "";

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5
#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM    13

const char* apiHost = "api.platerecognizer.com";
const String apiEndpoint = "/v1/plate-reader/";

bool initCamera() {
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
  config.jpeg_quality = 12; // Qualidade JPEG (0-63, menor é melhor)
  config.fb_count = 1;                 
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Falha na inicialização da câmera com erro 0x%x\n", err);
    return false;
  }

  Serial.println("Câmera inicializada com sucesso.");
  return true;
}

void captureAndSendImage() {
  Serial.println("Capturando imagem...");
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Falha ao capturar imagem do framebuffer.");
    return;
  }

  Serial.printf("Imagem capturada! Tamanho: %zu bytes\n", fb->len);

  WiFiClientSecure client;
  client.setInsecure();

  Serial.printf("Conectando ao host: %s\n", apiHost);

  if (!client.connect(apiHost, 443)) {
    Serial.println("Falha na conexão com o host!");
    esp_camera_fb_return(fb);
    return;
  }

  Serial.println("Conexão bem-sucedida.");

  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String head = "--" + boundary + "\r\n" +
                "Content-Disposition: form-data; name=\"upload\"; filename=\"image.jpg\"\r\n" +
                "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t contentLength = head.length() + fb->len + tail.length();

  client.println("POST " + apiEndpoint + " HTTP/1.1");
  client.println("Host: " + String(apiHost));
  client.println("Authorization: Token " + String(apiToken));
  client.println("Content-Length: " + String(contentLength));
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println();
  client.print(head);

  uint8_t *buf = fb->buf;
  size_t len = fb->len;
  size_t sent = 0;

  while (len > 0) {
    size_t to_send = (len > 1024) ? 1024 : len;
    client.write(buf + sent, to_send);
    sent += to_send;
    len -= to_send;
  }

  client.print(tail);
  esp_camera_fb_return(fb);

  Serial.println("Imagem enviada. Aguardando resposta...");

  long startTime = millis();

  while (client.connected() && !client.available() && millis() - startTime < 10000) { // Timeout aumentado para 10s
    delay(100);
  }

  String responseBody = "";
  bool headersEnded = false;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (!headersEnded) {
      if (line == "\r") {
        headersEnded = true;
      }
    } else {
      responseBody += line;
    }
  }
  client.stop();

  if (responseBody.length() > 0) {
    Serial.println("Resposta da API recebida:");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, responseBody);

    if (error) {
      Serial.print("Falha ao analisar JSON: ");
      Serial.println(error.c_str());
      return;
    }

    JsonArray results = doc["results"];
    if (!results.isNull() && results.size() > 0) {
      const char* plate = results[0]["plate"]; 

      if (plate) {
        Serial.println("======================================");
        Serial.print("PLACA RECONHECIDA: ");
        Serial.println(plate);
        Serial.println("======================================");
      } else {
        Serial.println("Nenhuma placa encontrada no primeiro resultado.");
      }
    } else {
      Serial.println("Nenhum resultado de placa retornado pela API.");
    }
  } else {
    Serial.println("Nenhuma resposta recebida do servidor ou timeout.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nIniciando sistema de monitoramento de estacionamento...");

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  if (!initCamera()) {
    Serial.println("ERRO FATAL: Falha ao inicializar a câmera. Reiniciando em 10s...");
    delay(10000);
    ESP.restart();
  }

  Serial.println("\nSistema pronto. Pressione a tecla 'Enter' no Monitor Serial para capturar e processar uma imagem.");
}

void loop() {
  if (Serial.available() > 0) {
    char incomingChar = Serial.read();

    if (incomingChar == '\n' || incomingChar == '\r') {
      while(Serial.available() > 0) { Serial.read(); }
      
      captureAndSendImage();
      Serial.println("\nSistema pronto. Pressione a tecla 'Enter' para capturar e processar uma imagem.");
    }
  }
}
