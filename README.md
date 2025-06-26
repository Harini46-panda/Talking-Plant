Talking Plant

The "Talking Plant" is an IoT-based intelligent plant monitoring system developed using the ESP32 microcontroller. It continuously tracks environmental parameters such as "presence of gas","movement nearby" ,"temperature", and "humidity" using connected sensors, helping users understand when their plant needs care.

Features

Temperature and Humidity Monitoring:
  Uses DHT11 sensor to read and display real-time environmental conditions

Gas leakage montioring:
  Uses MQT11 sensor to detect the leakage of gas in that environment

Motion Detection:
   Uses PIR sensor to detect the motion nearby plant

ESP32 Powered:
  Runs on ESP-IDF or Arduino framework, ideal for IoT applications

Serial Monitor Output:
  Sensor readings are printed to the serial console for real-time monitoring

Technologies Used

Microcontroller:ESP32
Sensors:
DHT11 (Temperature + Humidity)
PIR sensor
MQT11 sensor
Programming Language:C (ESP-IDF)
IDE/Tools:Arduino IDE /ESP-IDF CLI
Communication:USB Serial Monitor

Hardware Requirements

* ESP32 Development Board
* DHT11 Sensor
* PIR Sensor
* MQT11 Sensor
* Breadboard + Jumper Wires
* USB Cable for Programming

System Overview

1. The ESP32 reads values from the DHT11 and MQT11,PIR sensors.
2. Based on thresholds, the system determines the plant's condition.
3. Readings are displayed on the serial monitor, informing the user whether the plant needs watering or environmental adjustments.
4. The readings are also logged into the mcap file in its format.
5. The mcap data was decoded and the sensor readings were printed as an output in the serial monitor.

Example Output

Temperature: 27°C  Humidity: 45%  
Motion: Motion detected, Somebody here? I am so glad to meet you!
Air is fresh and clean, No gas leakage detected


