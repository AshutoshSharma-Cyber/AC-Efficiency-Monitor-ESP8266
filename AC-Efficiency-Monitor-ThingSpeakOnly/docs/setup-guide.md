# Setup Guide - ThingSpeak Only Version

## 1) Hardware Wiring
Follow the pinout in `wiring/pinout-table.md`.

- Use 3.3V for power to DHT22 and OLED.
- Add 10kΩ pull-up resistors to each DHT22 DATA line if module does not include them.
- Use 220Ω resistor for the LED.

## 2) Arduino IDE Setup
1. Install ESP8266 board package (Boards Manager -> search ESP8266).
2. Install libraries: DHT (Adafruit), Adafruit GFX, Adafruit SSD1306, ThingSpeak.
3. Select **Tools -> Board -> NodeMCU 1.0 (ESP-12E Module)**.

## 3) Configure the Firmware
Open `firmware/AC_Monitor_ThingSpeak_Only.ino`.
Edit the top of the file:
- Set `WIFI_SSID` and `WIFI_PASS`.
- Set `THINGSPEAK_CHANNEL_ID` and `THINGSPEAK_WRITE_KEY`.

## 4) Upload and Test
- Upload the sketch.
- Open Serial Monitor at 115200 baud to view logs.
- After startup, the IP address will display and ThingSpeak updates will appear every 15s.

## 5) ThingSpeak Channel
Make sure your ThingSpeak channel has fields:
1. Intake Temperature
2. Outlet Temperature
3. Intake Humidity
4. Outlet Humidity
Enable Status field for diagnostic messages.
