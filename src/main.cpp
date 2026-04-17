#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <M5GFX.h>
#include <M5UnitOLED.h>
#include <DHT.h>

#define DHTPIN 33 // pin 33 for DHT11 sensor data pin
#define DHTTYPE DHT11 // sensor type DHT11

M5UnitOLED oled(27, 26, 400000); // pins 27/26 for i2c

const char* ssid = "YOUR_SSID_HERE"; // enter your wifi ssid here
const char* password = "YOUR_PASSWORD_HERE"; // enter your wifi password here
const int8_t tzOffset = 0; // enter your UTC timezone offset here
DHT dht(DHTPIN, DHTTYPE);

void dhtloop() {
        float humidity = dht.readHumidity();
        float tempF = dht.readTemperature(true); // true for F
        float correctedTemp = tempF - (tempF * 0.05); // subtract 5% to correct error, may vary sensor to sensor
        char tempStr[20];
        char humStr[20];
        if (!isnan(humidity) && !isnan(tempF)) {
                snprintf(tempStr, sizeof(tempStr), "%.1f F", correctedTemp);
                snprintf(humStr, sizeof(humStr), "%.0f%%", humidity);
                oled.fillRect(0, 21, oled.width(), 43); // 21 comes from time text starting at a y pos of 5, plus 16 pixels for font size - 43 is the remainder of the screen (64-21)
                oled.drawString(tempStr, ((oled.width() - oled.textWidth(tempStr)) / 2), (oled.fontHeight() + 10));
                oled.drawString(humStr, ((oled.width() - oled.textWidth(humStr)) / 2), ((oled.fontHeight() + 10) + oled.fontHeight() + 5));
                oled.display();
                Serial.println("Updated temp/humidity");
        }else {
                snprintf(tempStr, sizeof(tempStr), "Error");
                snprintf(humStr, sizeof(humStr), "Error");
                oled.fillRect(0, 21, oled.width(), 43); // same number reasoning as above
                oled.drawString(tempStr, ((oled.width() - oled.textWidth(tempStr)) / 2), (oled.fontHeight() + 10));
                oled.drawString(humStr, ((oled.width() - oled.textWidth(humStr)) / 2), ((oled.fontHeight() + 10) + oled.fontHeight() + 5));
                oled.display();
                Serial.println("DHT11 read error");
        }
}

void timeloop() {
        static bool firstRun = true;
        struct tm t;
        if (!getLocalTime(&t)) {
                oled.drawString("No time", 5, (oled.textWidth("No time")));
                oled.display();
                return;
        }
        int hours = t.tm_hour;
        static int minutes;
        bool isPM = hours >= 12;
        int displayHour = hours % 12;
        if (displayHour == 0) {
                displayHour = 12;
        }
        if (firstRun || (minutes != t.tm_min)) {
                minutes = t.tm_min;
                char timeStr[12];
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d %s", displayHour, minutes, isPM ? "PM" : "AM");
                int centerX = ((oled.width() / 2) - (oled.textWidth(timeStr) / 2));
                oled.fillRect(0, 5, oled.width(), oled.fontHeight());
                oled.drawString(timeStr, centerX, 5);
                oled.display();
                Serial.println("Updated time");
                if (firstRun) {
                        firstRun = false;
                }
        }
}

void setup() {
        Serial.begin(115200);
        delay(1000);
        WiFi.begin(ssid, password);
        dht.begin();
        Serial.println("DHT11 initialized");
        Serial.println("Connecting to WiFi");
        oled.setTextSize(2);
        while (WiFi.status() != WL_CONNECTED) {
                delay(250);
                Serial.print("...");
        }
        Serial.println("Connected!");
        configTime(tzOffset * 3600, 0, "pool.ntp.org", "time.nist.gov");
        oled.init();
        oled.setRotation(1);
        oled.fillScreen(0);
}

void loop() {
        static unsigned long last = 0;
        unsigned long now = millis();
        if (now - last >= 15000) { // poll DHT every 15 seconds
                last = now;
                dhtloop();
        }
        timeloop();
        delay(20);
}
