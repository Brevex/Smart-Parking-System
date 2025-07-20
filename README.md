<h1 align = "center">Smart Parking System</h1><br>

<h2> &#128269; About the project </h2>

<p>ESP32-based parking system with license plate recognition. The system is connected in real time to the AdafruitIO IoT platform via MQTT and uses the Plate Recognizer platform to perform OCR on license plates. The system is divided into two separate circuits: one responsible for card reading and opening the barrier, and the other exclusively for capturing license plate images.</p><br>

<h2> &#128302; Technologies Used </h2><br>

<p align="center">
  <a href="https://github.com/syvixor/skills-icons">
	  <img src="https://skills.syvixor.com/api/icons?i=arduino,mqtt" alt="Skills">
  </a>
</p>

<br><h2> &#128161; How the Circuit Works? </h2>

<br><p align="center">
  <img src="https://github.com/Brevex/CAM-Parking-System/blob/5eeb93a8659820199190b52fa993e4daeacc02b4/readme_images/circuit.png" alt="circuit">
</p><br>

<ul>
  <li>Reads the card and checks if it has a valid ID in the system. If authorized, a blue LED will light up and a gate represented by a servo motor will open. If the card is declined 3 times, an alarm will sound and a red LED will flash.</li>
  <li>When the car approaches the space, an infrared sensor will detect its presence and start a counter. If x amount of time passes, an image will be captured and sent to the Plate Recognizer platform.</li>
  <li>The data collected by the sensors and camera will be sent in real time via MQTT to the AdafruitIO platform</li>
</ul>

<br><h2> &#128295; Circuit Assembly </h2>

<br><div align="center">

  <h3>ESP­-WROOM­-32</h3>   
  
  | Compenent               | Component Pin | ESP32 Pin |
  |:-----------------------:|:-------------:|:---------:|
  | RFID-RC522              | SDA           | GPIO05    |
  | RFID-RC522              | SCK           | GPIO18    |
  | RFID-RC522              | MOSI          | GPIO23    |
  | RFID-RC522              | MISO          | GPIO19    |
  | RFID-RC522              | RST           | GPIO27    |
  | RFID-RC522              | VCC           | 3.3V      |
  | LCD 16x2 (I2C)          | SDA           | GPIO21    |
  | LCD 16x2 (I2C)          | SCL           | GPIO22    |
  | LCD 16x2 (I2C)          | VCC           | 5V        |
  | Servo                   | SCL           | GPIO13    |
  | Servo                   | VCC           | 5V        |
  | Buzzer                  | +             | GPIO26    |
  | LED (Blue)              | +             | GPIO12    |
  | LED (Red)               | +             | GPIO14    |

  <h3>ESP­32-S3-CAM</h3>   
  
  | Compenent               | Component Pin | ESP32 Pin |
  |:-----------------------:|:-------------:|:---------:|
  | OV2640                  | XCLK          | GPIO15    |
  | OV2640                  | SIOD          | GPIO04    |
  | OV2640                  | SIOC          | GPIO05    |
  | OV2640                  | Y9            | GPIO16    |
  | OV2640                  | Y8            | GPIO17    |
  | OV2640                  | Y7            | GPIO18    |
  | OV2640                  | Y6            | GPIO12    |
  | OV2640                  | Y5            | GPIO10    |
  | OV2640                  | Y4            | GPIO08    |
  | OV2640                  | Y3            | GPIO09    |
  | OV2640                  | Y2            | GPIO11    |
  | OV2640                  | VSYNC         | GPIO06    |
  | OV2640                  | HREF          | GPIO07    |
  | OV2640                  | PCLK          | GPIO13    |
  | IR-Sensor               | OUT           | GPIO40    |
  | IR-Sensor               | VCC           | 5V        |
  
</div>

<br><h3 align = "center"> - By <a href = "https://www.linkedin.com/in/breno-barbosa-de-oliveira-810866275/" target = "_blank">Breno</a> - </h3>
