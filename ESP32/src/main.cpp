#include <WiFi.h>
#include <Wire.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <FirebaseESP32.h>
#include <LiquidCrystal_I2C.h>

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"
#include "./credentials.h"

#define GREEN_LED 26
#define RED_LED 27
#define BUZZER 34
#define SENSOR_FC51 32
#define SERVO_PIN 13
#define RST_PIN 33
#define SS_PIN 4

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servoMotor;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK                    = false;
const char* validIDs[]           = {"6387FC94"};
const int maxInvalidAttempts     = 3;

int emptyParkingSpaces = 1;
int invalidAttempts    = 0;
int alarmActvations    = 0;
int cardReadAttempts   = 0;
String lastID          = "";
String lastInvalidID   = "";
bool wifiConnected     = false;

void connectWifi() 
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
}

bool getWifiStatus() 
{
    return WiFi.status() == WL_CONNECTED;
}

void connectFirebase() 
{
    config.host = DATABASE_URL;
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    if (Firebase.ready()) 
    {
        Serial.println("Connected to Firebase");
    } 
    else { Serial.println("Firebase not connected"); }
}

void sendDataToFirebase() 
{
    if (Firebase.ready()) 
    {
        FirebaseJson json;
        json.set("/vagasDisponiveis", emptyParkingSpaces);
        json.set("/ultimoID", lastID.c_str());
        json.set("/tentativasInvalidas", invalidAttempts);
        json.set("/ativacoesAlarme", alarmActvations);
        json.set("/ultimoIDInvalido", lastInvalidID.c_str());
        json.set("/wifiConectado", getWifiStatus());
        Firebase.updateNode(fbdo, "/", json);
    }
}

void updateParkingLotStatus() 
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Vagas: ");
    lcd.print(emptyParkingSpaces);
}

void alarm() 
{
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(5000);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
    alarmActvations++;
    sendDataToFirebase();
}

void moveServoSlowly(int targetPosition)
{
    int currentPos = servoMotor.read();
    int increment = (targetPosition > currentPos) ? 1 : -1;
    
    for (int pos = currentPos; pos != targetPosition; pos += increment)
    {
        servoMotor.write(pos);
        delay(15);  
    }
    servoMotor.write(targetPosition); 
}

void accessFree(String id) 
{
    lastID = id;
    emptyParkingSpaces--;
    updateParkingLotStatus();
    sendDataToFirebase();
    moveServoSlowly(90);
    unsigned long startTime = millis();
    
    while (millis() - startTime < 15000) 
    {
        if (digitalRead(SENSOR_FC51) == LOW) 
        {
            emptyParkingSpaces = 0;
            break;
        }
        delay(100);
    }
    moveServoSlowly(0);
}

void accessDenied() 
{
    invalidAttempts++;
    alarm();
    sendDataToFirebase();
}

String readRFIDCard() 
{
    String id = "";
    
    for (byte i = 0; i < rfid.uid.size; i++) 
    {
        id += String(rfid.uid.uidByte[i], HEX);
    }
    id.toUpperCase();
    return id;
}

void cardValidate(String id) 
{
    bool isValid = false;
    
    for (int i = 0; i < (sizeof(validIDs) / sizeof(validIDs[0])); i++) 
    {
        if (id == validIDs[i]) 
        {
            isValid = true;
            break;
        }
    }

    if (isValid) 
    {
        accessFree(id);
        cardReadAttempts = 0; 
    } 
    else 
    {
        lastInvalidID = id;
        cardReadAttempts++;
        
        if (cardReadAttempts >= 3) 
        {
            accessDenied();
            cardReadAttempts = 0; 
        } 
        else 
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TENTE NOVAMENTE");
        }
    }
}

void parkingLotMonitor() 
{
    int sensorState = digitalRead(SENSOR_FC51);
    
    if (sensorState == LOW) 
    {
        emptyParkingSpaces = 0;
    } 
    else { emptyParkingSpaces = 1; }
    
    updateParkingLotStatus();
    sendDataToFirebase();
}

void setup() 
{
    Serial.begin(115200);

    connectWifi();
    connectFirebase();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(SENSOR_FC51, INPUT);
    servoMotor.attach(SERVO_PIN);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("SISTEMA DE ESTACIONAMENTO");

    SPI.begin();
    rfid.PCD_Init();
    servoMotor.write(0);

    updateParkingLotStatus();
}

void loop() 
{
    parkingLotMonitor();

    if (emptyParkingSpaces > 0) 
    {
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) 
        {
            String cardID = readRFIDCard();
            cardValidate(cardID);
            rfid.PICC_HaltA();
        }
    }
}
