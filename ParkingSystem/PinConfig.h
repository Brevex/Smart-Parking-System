/*
* ESP32-WROOM-32D
* 
* RFID MFRC522:
* SDA(5), SCK(18), MOSI(23), MISO(19), RST(27) - 3.3V
*
* LCD 16x2 I2C:
* SCL(22), SDA(21) - 5V
*
* Servo: 
* 5V
*/

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define SS_PIN   5
#define RST_PIN  27 

#define SERVO_PIN      13 
#define BLUE_LED_PIN   12 
#define RED_LED_PIN    14 
#define BUZZER_PIN     26

#endif 
