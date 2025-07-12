#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "esp_camera.h"
#include "credentials.h" 
#include "camera_pins.h"

const int irSensorPin = 40;                 
const long detectionThreshold = 5000; 

unsigned long objectDetectedTime = 0;       
bool objectPresent = false;                 
bool imageHasBeenCaptured = false;          

const char* apiHost = "api.platerecognizer.com";
const String apiEndpoint = "/v1/plate-reader/";

#define ADAFRUIT_IO_SERVER      "io.adafruit.com"
#define ADAFRUIT_IO_SERVERPORT  1883

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, ADAFRUIT_IO_SERVER, ADAFRUIT_IO_SERVERPORT, ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY);
Adafruit_MQTT_Publish vagaStatusFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/vaga-status");
Adafruit_MQTT_Publish vagaPlacaFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/vaga-placa");

String lastPlate = "";

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Conectando ao MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Falha ao conectar ao MQTT. Tentando novamente em 5 segundos...");
    mqtt.disconnect();
    delay(5000); 
    retries--;

    if (retries == 0) {
      Serial.println("Não foi possível conectar ao MQTT. Reiniciando...");
      ESP.restart();
    }
  }

  Serial.println("MQTT Conectado!");
}

void sendToAdafruitIO(const char* status, const char* plate) {
  MQTT_connect(); 

  Serial.printf("Enviando para Adafruit IO -> Status: %s, Placa: %s\n", status, plate);

  if (!vagaStatusFeed.publish(status)) {
    Serial.println("Falha ao publicar o status da vaga.");
  } else {
    Serial.println("Status da vaga publicado com sucesso!");
  }

  if (!vagaPlacaFeed.publish(plate)) {
    Serial.println("Falha ao publicar a placa.");
  } else {
    Serial.println("Placa publicada com sucesso!");
  }
}


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
  config.jpeg_quality = 12; // 0-63, menor = maior qualidade
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
  Serial.println("Limpando buffer da câmera para obter um quadro novo...");
  camera_fb_t *fb_clear = esp_camera_fb_get();

  if (fb_clear) {
    esp_camera_fb_return(fb_clear);
  }

  Serial.println("Capturando imagem atual...");
  camera_fb_t *fb = esp_camera_fb_get(); 

  if (!fb) {
    Serial.println("Falha ao capturar imagem do framebuffer.");
    return;
  }

  Serial.printf("Imagem capturada! Tamanho: %zu bytes\n", fb->len);

  WiFiClientSecure clientSecure;
  clientSecure.setInsecure(); 

  Serial.printf("Conectando ao host: %s\n", apiHost);

  if (!clientSecure.connect(apiHost, 443)) {
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

  clientSecure.println("POST " + apiEndpoint + " HTTP/1.1");
  clientSecure.println("Host: " + String(apiHost));
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

  Serial.println("Imagem enviada. Aguardando resposta...");

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
        
        lastPlate = String(plate);
        sendToAdafruitIO("Ocupado", plate);

      } else {
        Serial.println("Nenhuma placa encontrada no primeiro resultado.");
        sendToAdafruitIO("Ocupado", "N/A");
        lastPlate = "N/A";
      }
    } else {
      Serial.println("Nenhum resultado de placa retornado pela API.");
      sendToAdafruitIO("Ocupado", "Erro API");
      lastPlate = "Erro API";
    }
  } else {
    Serial.println("Nenhuma resposta recebida do servidor ou timeout.");
    sendToAdafruitIO("Ocupado", "Timeout");
    lastPlate = "Timeout";
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nIniciando sistema de monitoramento de estacionamento...");

  pinMode(irSensorPin, INPUT_PULLUP);

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
  
  sendToAdafruitIO("Livre", "");

  Serial.println("\nSistema pronto. Aguardando veículo...");
}

void loop() {
  MQTT_connect();
  mqtt.processPackets(100);

  objectPresent = (digitalRead(irSensorPin) == LOW);

  if (objectPresent && objectDetectedTime == 0) {
    Serial.println("Veículo detectado. Iniciando contagem...");
    objectDetectedTime = millis(); 
    imageHasBeenCaptured = false;  
  }

  if (objectPresent && objectDetectedTime > 0 && !imageHasBeenCaptured) {
    if (millis() - objectDetectedTime > detectionThreshold) {
      Serial.println("Veículo estável. Acionando a captura.");
      
      captureAndSendImage();
      
      imageHasBeenCaptured = true;
      
      Serial.println("\nCaptura concluída. Aguardando o veículo sair para rearmar o sistema.");
    }
  }

  if (!objectPresent) {
    if (objectDetectedTime > 0) {
      Serial.println("Veículo saiu. Sistema rearmado e pronto para a próxima detecção.");
      sendToAdafruitIO("Livre", "");
      lastPlate = "";
    }
    objectDetectedTime = 0;
  }
  
  delay(100);
}
