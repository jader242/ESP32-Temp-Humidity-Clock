#include <Arduino.h>
#include <M5UnitOLED.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DHT.h>

#define DHTPIN 33 // pin 33
#define DHTTYPE DHT11 // sensor type

M5UnitOLED oled(27, 26, 400000); // use pins 26 and 27 for i2c sda/scl, 400kHz (mandatory to put this)

const char* ssid = "YOUR_WIFI_SSID"; // change this to your wifi ssid
const char* password = "YOUR_WIFI_PASSWORD"; // change this to your wifi password
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org", -25200, 60000); //change second arg for your timezone
DHT dht(DHTPIN, DHTTYPE);
int firstRun = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.begin(ssid, password);
  dht.begin();
  Serial.println("DHT11 Initialized");
  Serial.println("Connecting to WiFi");
  oled.setTextSize(2);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print("...");
  }
  Serial.println("Connected");
  timeClient.begin();
  oled.init();
  oled.setRotation(1);
  oled.fillScreen(0);
  const char* welcome = "Welcome :)";
  oled.drawString(welcome, oled.width() / 2 - oled.textWidth(welcome) / 2, oled.height() / 2 - oled.fontHeight() / 2);
  delay(2000);
  oled.fillScreen(0);
}

void loop() {
  timeClient.update();
  static int lastMinutes = -1;
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  char timeStr[12];
  if (hours == 12) {
    sprintf(timeStr, "%02d:%02d PM", hours, minutes);
  }else if (hours == 0) {
    sprintf(timeStr, "%02d:%02d AM", (hours + 12), minutes);
  }else {
    if (hours < 12) {
      sprintf(timeStr, "%02d:%02d AM", hours, minutes);
    }else {
      sprintf(timeStr, "%02d:%02d PM", (hours - 12), minutes);
    }
  }
  static float lastTemperatureF = NAN;
  static float lastHumidity = NAN;
  float humidity = dht.readHumidity();
  float temperatureF = dht.readTemperature(true); //true for F
  float correctedTemp = temperatureF - (temperatureF * 0.05); // my sensor reads a bit high, YMMV
  oled.setTextSize(2);
  int time_yoffset = oled.fontHeight();
  char tempStr[20];
  char humidityStr[20];
  if (!isnan(humidity) && !isnan(temperatureF)) { // check if humidity and temp are numbers
    sprintf(tempStr, "%.1f F", correctedTemp);  // one decimal place with 'F', subtract 5% for accuracy
    sprintf(humidityStr, "%.0f%%", humidity); // no decimal place with % sign
  }else {
    sprintf(tempStr, "Temp Error");
    sprintf(humidityStr, "Humidity Error");
  }
  int timeX = (oled.width() - oled.textWidth(timeStr)) / 2;
  int tempX = (oled.width() - oled.textWidth(tempStr)) / 2;
  int humX = (oled.width() - oled.textWidth(humidityStr)) / 2;
  if (hours >= 6 && hours < 21) { // need to change this in the future so its not writing to i2c bus every loop
    oled.setBrightness(128);
  }else {
    oled.setBrightness(64);
  }
  if (firstRun == 0) {
    oled.drawString(timeStr, timeX, 5);
    oled.drawString(tempStr, tempX, (time_yoffset + 10));
    oled.drawString(humidityStr, humX, ((time_yoffset + 10) + oled.fontHeight() + 5));
    oled.display();
    firstRun++;
    lastTemperatureF = temperatureF;
    lastHumidity = humidity;
    Serial.println("First Run Complete"); // idk why tf this prints so many times lmao
  }
  if (minutes != lastMinutes) {
    int newHours = timeClient.getHours();
    int newMinutes = timeClient.getMinutes();
    lastMinutes = newMinutes;
    char newTimeStr[12];
    if (newHours == 12) {
      sprintf(newTimeStr, "%02d:%02d PM", newHours, newMinutes);
    }else if (newHours == 0) {
      sprintf(newTimeStr, "%02d:%02d AM", (newHours + 12), newMinutes);
    }else {
      if (newHours < 12) {
        sprintf(newTimeStr, "%02d:%02d AM", newHours, newMinutes);
      }else {
        sprintf(newTimeStr, "%02d:%02d PM", (newHours - 12), newMinutes);
      }
    }
    oled.fillRect(0, 5, oled.width(), oled.fontHeight());
    oled.drawString(newTimeStr, timeX, 5);
    oled.display();
    Serial.println("Updated Time");
  }
  if (!isnan(humidity) && !isnan(temperatureF)) {
    if (fabs(correctedTemp - lastTemperatureF) > 0.5) { // only update when temp fluctuates more than 0.5 degrees to avoid over reading from the dht
      lastTemperatureF = correctedTemp;
      oled.fillRect(0, (time_yoffset + 10), oled.width(), oled.fontHeight());
      oled.drawString(tempStr, tempX, (time_yoffset + 10));
      oled.display();
      Serial.println("Updated Temp");
    }
    if (fabs(humidity - lastHumidity) > 1.0) { // same thing here but if humidity fluctuates more than 1%
      lastHumidity = humidity;
      oled.fillRect(0, ((time_yoffset + 10) + (oled.fontHeight() + 5)), oled.width(), oled.fontHeight());
      oled.drawString(humidityStr, humX, ((time_yoffset + 10) + oled.fontHeight() + 5));
      oled.display();
      Serial.println("Updated Humidity");
    }
  }
 }
