# AC Cooling Efficiency Monitor — ThingSpeak Only

This repository contains the **ThingSpeak-only** firmware and documentation for the
AC Cooling Efficiency Monitor project using **NodeMCU (ESP8266)**, **two DHT22 sensors**,
an **SSD1306 OLED**, a **buzzer**, and a **red LED**.

This simplified version uploads sensor readings and status messages to ThingSpeak,
and provides local alerts via OLED / buzzer / LED. It does **not** include IFTTT or Telegram.

## Included
- firmware/AC_Monitor_ThingSpeak_Only.ino  — main Arduino sketch
- wiring/pinout-table.md                  — wiring & pin mapping
- docs/setup-guide.md                     — setup and upload instructions
- libraries/required-libraries.txt       — list of required libraries

## Usage
1. Install required libraries listed in `libraries/required-libraries.txt`.
2. Open `firmware/AC_Monitor_ThingSpeak_Only.ino` in Arduino IDE.
3. Edit WiFi credentials and ThingSpeak channel ID / write key at the top of the sketch.
4. Select **Tools → Board → NodeMCU 1.0 (ESP-12E Module)** and correct COM port.
5. Upload the sketch, and open Serial Monitor (115200) to follow logs.
6. Check your ThingSpeak channel for live updates.

