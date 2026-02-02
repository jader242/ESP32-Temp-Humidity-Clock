My first ESP32 Project

Just a simple clock that displays temperature and humidity, syncs ntp, and also dims the screen during bedtime hours.

**Hardware used:**
- ESP32-Wroom devkit
- DHT11 sensor
- M5Stack Unit OLED Screen

**Future goals:**
- ✔  reduce screen flicker - No more 15 second delay and full screen refresh each loop. Now it checks for differences in the time, temp, and humidity and refreshes each component accordingly
- implement multiple screens and a way to switch between them (ie m5stack dual button unit or scroll unit)
- add a buzzer for an alarm clock
- change auto brightness logic to not hammer the i2c bus each loop

![ESP32 Temp/Humidity Clock Display](/assets/display.jpeg)

**How to Build (if you have all the same hardware and want to try it)**
- Git clone repo
- Add your WiFi SSID and Password to src/main.cpp (the lines are commented)
- Change timezone for NTP if necessary (this line is also commented)
- Have PlatformIO VS Code extension or CLI
- 'pio run -t upload'
