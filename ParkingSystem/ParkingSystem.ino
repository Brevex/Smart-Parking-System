#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

#include "PinConfig.h" 

#define SERVO_PARADO            90
#define SERVO_VELOCIDADE_ABRIR  180 
#define SERVO_VELOCIDADE_FECHAR 0   
#define TEMPO_MOVIMENTO_CANCELA 2500 

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo cancelaServo;
String cartaoValido = "73 82 33 1C"; 
int invalidScanCount = 0;

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
}

void loop() {
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