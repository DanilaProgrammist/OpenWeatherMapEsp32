#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BTN_DEB 50 
#define BUTTON_PIN 23
Adafruit_SH1107 display = Adafruit_SH1107(128, 128, &Wire);
const char* ssid = "WiFi ssid";
const char* password = "WiFi password";
const char* API_KEY = "OpenWeatherMap api key";
const unsigned char thermometer [] PROGMEM = {
	0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 
	0x02, 0x40, 0x02, 0x40, 0x04, 0x20, 0x05, 0xa0, 0x09, 0x90, 0x05, 0xa0, 0x06, 0x60, 0x03, 0xc0
};
const unsigned char drop [] PROGMEM = {
	0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x08, 0x10, 0x10, 0x08, 
	0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x1c, 0x08, 0x14, 0x08, 0x0b, 0x10, 0x0c, 0x30, 0x03, 0xc0
};
const unsigned char barometer [] PROGMEM = {
	0x07, 0xe0, 0x1d, 0xb8, 0x30, 0x0c, 0x70, 0x0e, 0x41, 0x82, 0xc1, 0x83, 0x81, 0x81, 0xc1, 0x83, 
	0xc3, 0xc3, 0x83, 0xc1, 0xc1, 0x83, 0x40, 0x02, 0x70, 0x0e, 0x30, 0x0c, 0x1c, 0x38, 0x07, 0xe0
};
const char* cities[] ={
  "Moscow",
  "Baku",
  "Seattle",
  "London",
  "Sydney"
};
int currentCity = 0;
static bool pState = false;
long tmr;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(21,22);
  Wire.setClock(40000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print("Connecting");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("WiFi connected");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.clearDisplay(); 
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (buttonClick()){
    Serial.println("Btn Click");
    currentCity++;
    currentCity = (currentCity > 4 ? 0 : currentCity);
    getResponce(cities[currentCity]);
  }
}
void getResponce(String city){
  HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=") + city + "&appid=" + API_KEY + "&units=metric";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200){
    String payload = http.getString();
    Serial.println(payload);
    StaticJsonDocument<1024> json;
    deserializeJson(json, payload);
    int temp = json["main"]["temp"];
    int press = json["main"]["pressure"];
    int humi = json["main"]["humidity"];
    long dt = json["dt"];
    long timezone = json["timezone"];
    long localtime = timezone+dt;
    int hours = (localtime % 86400L) / 3600;
    int minutes = ((localtime % 86400L)-(hours * 3600))/60;
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hours, minutes);
    press = press * 0.75;
    http.end();
    drawScreen(city, timeStr, temp, press, humi);
    
  }
  else {
    Serial.print("Error: ");
    Serial.println(httpCode);
  }
}
void drawScreen(String city, String time, int temp, int press, int humi){
  display.clearDisplay();
  int16_t x1, y1, x2, y2;
  uint16_t h, w, h1, w1;
  
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.println(city);


  display.setTextSize(2);

  display.getTextBounds(time, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(display.width()/2 - w/2, 30);
  display.println(time);


  display.drawLine(0, 50, 127, 50, SSD1306_WHITE);


  display.setTextSize(1);
  display.setCursor(5, 54);
  display.println("TEMP");
  display.setCursor(5, 64);
  String _temp = "TEMP";
  display.getTextBounds(_temp, 0, 0, &x1, &y1, &w, &h);
  display.getTextBounds(String(temp), 0, 0, &x2, &y2, &w1, &h1);
  display.setCursor(w/2 - w1/2, 64);
  display.print(temp);
  display.print("C");


  display.setCursor(48, 54);
  display.println("HUM");
  display.setCursor(48, 64);
  display.print(humi);
  display.print("%");


  display.setCursor(90, 54);
  display.println("PRES");
  display.setCursor(90, 64);
  display.print(press);
  display.print("mm.");


  display.drawBitmap(8,78, thermometer, 16,16,SSD1306_WHITE);
  display.drawBitmap(49,78, drop, 16,16,SSD1306_WHITE);
  display.drawBitmap(98,78, barometer, 16,16,SSD1306_WHITE);

  display.display();
}
bool buttonClick(){
    bool state = !digitalRead(BUTTON_PIN);
    if (state && !pState && millis() - tmr  > 50){
      pState = true;
      tmr = millis();
      return true;
    }
    if (!state && pState){
      pState = false;
      return false;
    }
    return false;
}
