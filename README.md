# smart_clock
# 🕒 ESP32 Smart Room Clock & Temperature Monitor

A compact smart room clock and weather monitor powered by an **ESP32 DevKit V1** microcontroller, a **0.96" OLED display (SSD1306)**, and a **DHT11 sensor**. The project connects to Wi-Fi, automatically synchronizes accurate time using NTP servers, and displays room temperature with non-blocking screen switching.

---

## ✨ Features
* **Automatic Time Synchronization:** Fetches precise time and date via the internet (NTP protocol) without needing an external RTC hardware module.
* **Climate Monitoring:** Measures indoor temperature using the DHT11 sensor.
* **Adaptive Interface:** Smoothly alternates (using non-blocking `millis()`) between the weather screen and the time/date screen every 5 seconds.
* **Clean Code Structure:** Written in pure C++ for the Arduino IDE using optimized, industry-standard libraries.

---

## 🛠 Components Used
* **Microcontroller:** ESP32 DevKit V1
* **Display:** 0.96" OLED Display (I2C, SSD1306, 128x64 resolution)
* **Sensor:** DHT11 (Temperature & Humidity Sensor)
* **Connectivity:** Built-in Wi-Fi (ESP32)
  
