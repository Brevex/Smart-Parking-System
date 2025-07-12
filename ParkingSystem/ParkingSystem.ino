#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "pin_config.h" 
#include "credentials.h"

#define SERVO_PARADO            90
#define SERVO_VELOCIDADE_ABRIR  180 
#define SERVO_VELOCIDADE_FECHAR 0   
#define TEMPO_MOVIMENTO_CANCELA 2500 
#define ADAFRUIT_IO_SERVER      "io.adafruit.com"
#define ADAFRUIT_IO_SERVERPORT  1883

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo cancelaServo;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, ADAFRUIT_IO_SERVER, ADAFRUIT_IO_SERVERPORT, ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY);
Adafruit_MQTT_Publish rfidLidoFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/rfid-lido");
Adafruit_MQTT_Publish alarmeContagemFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/alarmes-contagem");

String cartaoValido = "73 82 33 1C"; 
int invalidScanCount = 0;
int alarmCount = 0;

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

void sendRfidToAdafruit(String uid) {
  MQTT_connect();
  Serial.print("Enviando RFID para Adafruit IO: ");
  Serial.println(uid);

  if (!rfidLidoFeed.publish(uid.c_str())) {
    Serial.println("Falha ao publicar o RFID.");
  } else {
    Serial.println("RFID publicado com sucesso!");
  }
}

void sendAlarmCountToAdafruit(int count) {
  MQTT_connect();
  Serial.print("Enviando contagem de alarmes para Adafruit IO: ");
  Serial.println(count);

  if (!alarmeContagemFeed.publish((int32_t)count)) {
    Serial.println("Falha ao publicar a contagem de alarmes.");
  } else {
    Serial.println("Contagem de alarmes publicada com sucesso!");
  }
}

void mostraMensagemInicial() {
  lcd.clear();
  delay(10);
  lcd.setCursor(0, 0);
  lcd.print("Aproxime o seu");
  lcd.setCursor(4, 1);
  lcd.print("cartao");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nConectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  cancelaServo.attach(SERVO_PIN, 500, 2500); 
  delay(100);
  cancelaServo.write(SERVO_PARADO); 
  
  delay(500);
  mostraMensagemInicial();
  Serial.println("Sistema de Estacionamento Inteligente Iniciado!");
  Serial.println("Servo posicionado em PARADO.");

  sendAlarmCountToAdafruit(alarmCount);
}

void loop() {
  MQTT_connect();
  mqtt.processPackets(100);

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.trim();
  uidString.toUpperCase();

  Serial.print("Cartao Detectado! UID: ");
  Serial.println(uidString);

  sendRfidToAdafruit(uidString);

  if (uidString == cartaoValido) {
    Serial.println("ACESSO PERMITIDO!");
    invalidScanCount = 0;

    digitalWrite(BLUE_LED_PIN, HIGH);
    lcd.clear();
    delay(10);
    lcd.setCursor(3, 0);
    lcd.print("Bem-vindo!");

    Serial.println("Abrindo cancela...");
    cancelaServo.write(SERVO_VELOCIDADE_ABRIR); 
    delay(TEMPO_MOVIMENTO_CANCELA); 

    Serial.println("Cancela aberta. Aguardando passagem...");
    cancelaServo.write(SERVO_PARADO);
    delay(5000); 

    Serial.println("Fechando cancela...");
    cancelaServo.write(SERVO_VELOCIDADE_FECHAR);
    delay(TEMPO_MOVIMENTO_CANCELA);

    Serial.println("Cancela fechada.");
    cancelaServo.write(SERVO_PARADO);
    digitalWrite(BLUE_LED_PIN, LOW);

  } else {
    Serial.println("ACESSO NEGADO!");
    invalidScanCount++;
    Serial.print("Tentativas invalidas consecutivas: ");
    Serial.println(invalidScanCount);

    if (invalidScanCount >= 3) {
      Serial.println("ALERTA! Multiplas tentativas invalidas.");
      
      alarmCount++;
      sendAlarmCountToAdafruit(alarmCount);

      lcd.clear();
      delay(10);
      lcd.setCursor(1, 0);
      lcd.print("Sistema em");
      lcd.setCursor(4, 1);
      lcd.print("Alerta!");
      
      tone(BUZZER_PIN, 1000); 
      delay(3000);
      noTone(BUZZER_PIN);
      
      invalidScanCount = 0;
    } else {
      digitalWrite(RED_LED_PIN, HIGH);
      lcd.clear();
      delay(10);
      lcd.setCursor(0, 0);
      lcd.print("Cartao Invalido");

      delay(3000);
      digitalWrite(RED_LED_PIN, LOW);
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  Serial.println("\nSistema pronto para a proxima leitura.");
  mostraMensagemInicial();
}
