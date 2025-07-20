<h1 align = "center">CAM Parking System</h1><br>

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

  <h3>Using ESP­-WROOM­-32</h3>   
  
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
  | Buzzer                  | pin2          | GPIO26    |
  | LED (Blue)              | anode         | GPIO12    |
  | LED (Red)               | anode         | GPIO14    |
  
</div>

<br><h3 align = "center"> - By <a href = "https://www.linkedin.com/in/breno-barbosa-de-oliveira-810866275/" target = "_blank">Breno</a> - </h3>
