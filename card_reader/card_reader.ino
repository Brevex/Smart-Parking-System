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

#define SERVO_STOPPED_POSITION      90
#define SERVO_OPENING_SPEED         180
#define SERVO_CLOSING_SPEED         0
#define BARRIER_MOVEMENT_TIME_MS    2500
#define ADAFRUIT_IO_SERVER          "io.adafruit.com"
#define ADAFRUIT_IO_SERVERPORT      1883

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo barrierServo;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, ADAFRUIT_IO_SERVER, ADAFRUIT_IO_SERVERPORT, ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY);
Adafruit_MQTT_Publish rfidReadFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/rfid-lido"); 
Adafruit_MQTT_Publish alarmCountFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUIT_IO_USERNAME "/feeds/alarmes-contagem"); 

String validCardUid = "73 82 33 1C";
int invalidScanCount = 0;
int alarmCount = 0;

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

void SendRfidToAdafruit(String uid) {
  MqttConnect();
  Serial.print("Sending RFID to Adafruit IO: ");
  Serial.println(uid);

  if (!rfidReadFeed.publish(uid.c_str())) {
    Serial.println("Failed to publish RFID.");
  } else {
    Serial.println("RFID published successfully!");
  }
}

void SendAlarmCountToAdafruit(int count) {
  MqttConnect();
  Serial.print("Sending alarm count to Adafruit IO: ");
  Serial.println(count);

  if (!alarmCountFeed.publish((int32_t)count)) {
    Serial.println("Failed to publish alarm count.");
  } else {
    Serial.println("Alarm count published successfully!");
  }
}

void DisplayInitialMessage() {
  lcd.clear();
  delay(10);
  lcd.setCursor(0, 0);
  lcd.print("Approach your");
  lcd.setCursor(4, 1);
  lcd.print("card");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
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

  barrierServo.attach(SERVO_PIN, 500, 2500);
  delay(100);
  barrierServo.write(SERVO_STOPPED_POSITION);

  delay(500);
  DisplayInitialMessage();
  Serial.println("Smart Parking System Started!");
  Serial.println("Servo positioned at STOPPED.");

  SendAlarmCountToAdafruit(alarmCount);
}

void loop() {
  MqttConnect();
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

  Serial.print("Card Detected! UID: ");
  Serial.println(uidString);

  SendRfidToAdafruit(uidString);

  if (uidString == validCardUid) {
    Serial.println("ACCESS GRANTED!");
    invalidScanCount = 0;

    digitalWrite(BLUE_LED_PIN, HIGH);
    lcd.clear();
    delay(10);
    lcd.setCursor(3, 0);
    lcd.print("Welcome!");

    Serial.println("Opening barrier...");
    barrierServo.write(SERVO_OPENING_SPEED);
    delay(BARRIER_MOVEMENT_TIME_MS);

    Serial.println("Barrier open. Waiting for passage...");
    barrierServo.write(SERVO_STOPPED_POSITION);
    delay(5000);

    Serial.println("Closing barrier...");
    barrierServo.write(SERVO_CLOSING_SPEED);
    delay(BARRIER_MOVEMENT_TIME_MS);

    Serial.println("Barrier closed.");
    barrierServo.write(SERVO_STOPPED_POSITION);
    digitalWrite(BLUE_LED_PIN, LOW);

  } else {
    Serial.println("ACCESS DENIED!");
    invalidScanCount++;
    Serial.print("Consecutive invalid attempts: ");
    Serial.println(invalidScanCount);

    if (invalidScanCount >= 3) {
      Serial.println("ALERT! Multiple invalid attempts.");

      alarmCount++;
      SendAlarmCountToAdafruit(alarmCount);

      lcd.clear();
      delay(10);
      lcd.setCursor(1, 0);
      lcd.print("System on");
      lcd.setCursor(4, 1);
      lcd.print("Alert!");

      tone(BUZZER_PIN, 1000);
      delay(3000);
      noTone(BUZZER_PIN);

      invalidScanCount = 0;
    } else {
      digitalWrite(RED_LED_PIN, HIGH);
      lcd.clear();
      delay(10);
      lcd.setCursor(0, 0);
      lcd.print("Invalid Card");

      delay(3000);
      digitalWrite(RED_LED_PIN, LOW);
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  Serial.println("\nSystem ready for next read.");
  DisplayInitialMessage();
}
