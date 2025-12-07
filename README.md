# 4TR3 Automated Pancake Machine Capstone Code
How it works:

‣ Uses X, Y, and Z axis stepper motors to precisely draw pancake shapes.

‣ A motor-controlled valve accurately dispenses batter only when needed.

‣ An onboard Nextion HMI allows users to choose from 3 preset modes:

 ⁃ Batch of 1–6 pancakes
 
 ⁃ Square design
 
 ⁃ Triangle design

Once a design is selected, the system automatically executes the full sequence to draw the shape.


Cloud Integration:

‣ Batter level is measured using a Time of Flight (ToF) sensor.

‣ Uploaded via ESP8266 (ESP-01) using the MQTT protocol.

A Node-RED cloud dashboard visualizes:

 ⁃ Real-time batter level
 
 ⁃ Historical data trends over time
 
 ⁃ Sends a warning when the batter level is low


Devices Used:

‣ Arduino Mega

‣ Stepper motors

‣ Time of Flight sensor

‣ Nextion HMI

‣ ESP-01 | MQTT | Node-RED
