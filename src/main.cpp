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

const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org", -25200, 60000); //change second arg for your timezone
DHT dht(DHTPIN, DHTTYPE);

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
  oled.fillScreen(0);
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();
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
  float humidity = dht.readHumidity();
  float temperatureF = dht.readTemperature(true); //true for F
  oled.setTextSize(2);
  int time_yoffset = oled.fontHeight();
  char tempStr[20];
  char humidityStr[20];
  if (!isnan(humidity) && !isnan(temperatureF)) { // check if humidity and temp are numbers
    sprintf(tempStr, "%.1f F", temperatureF);  // one decimal place with 'F'
    sprintf(humidityStr, "%.0f%%", humidity); // no decimal place with % sign
  }else {
    sprintf(tempStr, "Temp Error");
    sprintf(humidityStr, "Humidity Error");
  }
  int timeX = (oled.width() - oled.textWidth(timeStr)) / 2;
  int tempX = (oled.width() - oled.textWidth(tempStr)) / 2;
  int humX = (oled.width() - oled.textWidth(humidityStr)) / 2;
  if (hours >= 6 && hours < 21) {
    oled.setBrightness(128);
  }else {
    oled.setBrightness(64);
  }
  oled.drawString(timeStr, timeX, 5);
  oled.drawString(tempStr, tempX, (time_yoffset + 10));
  oled.drawString(humidityStr, humX, ((time_yoffset + 10) + oled.fontHeight() + 5));
  oled.display();
  Serial.println("Running...");
  delay(15000);
 }
