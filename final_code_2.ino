#include "FreeMono12pt7b.h"
#include "FreeMonoBold24pt7b.h"
#include "icon_weather.h"
#include "wifiConfig.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "DHT.h"
#include <SPI.h>
#include <RTClib.h>
#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <Arduino.h>

// ================== RTC DS3231 ==================
#define RTC_SDA 26
#define RTC_SCL 27
RTC_DS3231 rtc;

// ================== DHT22 ==================
#define DHTPIN  21 
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

// ================== MÀN HÌNH TFT ==================
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ================== NÚT NHẤN ==================
#define BTN1  33 
#define BTN2  32 
#define BTN3  12 

// ================== BUZZER ==================
#define BUZZER_PIN 25

// ================== WEATHER API ==================
String key = "81060d388bc149bb9ef60bcb33a66acf";
String latitude = "10.851584";
String longitude = "106.772874";
int thoitiet = 800;
int nhietdo;
int doam;

HTTPClient http;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200); // GMT+7

// ================== BIẾN BÁO THỨC ==================
int gio_baothuc = 6;
int phut_baothuc = 0;
bool select_time = false;
bool bao_thuc = false;

// ================== BIẾN NGẮT ==================
volatile bool btn1_flag = false;
volatile bool btn2_flag = false;
volatile bool btn3_flag = false;

// ================== TÊN THỨ ==================
static const char* daysOfTheWeek[] = {
  "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};

// ================== HÀM ==================
void creat_srceen(int gio, int phut, int giay, int ngay, float h, float t);
void updateWiFiIcon();
void iconthoitiet(int code, int gio);
void playbeep();

// ================== ISR NÚT ==================
void IRAM_ATTR isr_btn1() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last > 200) {
    btn1_flag = true;
    last = now;
  }
}
void IRAM_ATTR isr_btn2() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last > 200) {
    btn2_flag = true;
    last = now;
  }
}
void IRAM_ATTR isr_btn3() {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last > 200) {
    btn3_flag = true;
    last = now;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);  
  wifiConfig.begin(); 
  wifiConfig.run();

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN1), isr_btn1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN2), isr_btn2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN3), isr_btn3, FALLING);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  Wire.begin(RTC_SDA, RTC_SCL);
  if (!rtc.begin()) {
    Serial.println("Không tìm thấy DS3231!");
    tft.setCursor(20, 200);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.println("Loi RTC!");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(2025, 10, 14, 16, 12, 0));
  }

  timeClient.begin();
  Serial.println("Đang kết nối WiFi để đồng bộ NTP...");

 
}

// ================== LOOP ==================
void loop() {

 // ===== WEATHER API =====
  http.begin("http://api.weatherbit.io/v2.0/current?lat=" + latitude + "&lon=" + longitude + "&key=" + key);
  http.setTimeout(5000);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String data = http.getString();

    DynamicJsonDocument doc(1500);
    DeserializationError error = deserializeJson(doc, data);
    if (!error) {
      nhietdo = doc["data"][0]["temp"].as<int>();
      doam = doc["data"][0]["rh"].as<int>();
      thoitiet = doc["data"][0]["weather"]["code"].as<int>();
      
      Serial.print("Mã thời tiết: "); Serial.println(thoitiet);
    } else {
      Serial.print("Lỗi parse JSON: "); Serial.println(error.c_str());
    }
  } else {
    Serial.print("Lỗi HTTP: ");
    Serial.println(httpCode);
  }
  http.end();

  static bool synced = false;
  static unsigned long lastDHT = 0;
  static float h = 0, t = 0;
  DateTime now;

  wifiConfig.run();
  updateWiFiIcon();

  // Cập nhật thời gian
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    if (!synced && epochTime > 1000000000) {
      rtc.adjust(DateTime(epochTime));
      synced = true;
      Serial.println("✅ DS3231 synced with NTP!");
    }

    now = DateTime(epochTime);
  } else {
    synced = false;
    now = rtc.now();
  }

  int gio  = now.hour();
  int phut = now.minute();
  int giay = now.second(); 
  int ngay = now.day();
  int thang = now.month();
  int nam   = now.year();

  // Đọc DHT22 mỗi 2s
  if (WiFi.status() == WL_CONNECTED){
    h = doam;
    t = nhietdo;
  } 
  else
  if (millis() - lastDHT > 2000) {
    lastDHT = millis();
    h = dht.readHumidity();
    t = dht.readTemperature();
  }

  // Cập nhật màn hình (chỉ khi thay đổi giây)
  static int oldSec = -1;
  if (giay != oldSec) {
    oldSec = giay;
    creat_srceen(gio, phut, giay, ngay, h, t);
    iconthoitiet(thoitiet, gio);

    tft.setFont();
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.setCursor(20, 50);
    tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
    tft.setCursor(140, 50); tft.print(","); tft.print(ngay);
    tft.setCursor(180, 50); tft.print("/"); tft.print(thang);
    tft.setCursor(220, 50); tft.print("/"); tft.print(nam);
  }

  // Nút nhấn
  if (btn1_flag) { btn1_flag = false; select_time = !select_time; }
  if (btn2_flag) {
    btn2_flag = false;
    if (select_time) gio_baothuc = (gio_baothuc + 1) % 24;
    else phut_baothuc = (phut_baothuc + 1) % 60;

     tft.fillRoundRect(10, 140, 143, 30, 10, ILI9341_DARKGREY);
  }
  if (btn3_flag) { btn3_flag = false; bao_thuc = !bao_thuc; }

  // Hiển thị icon báo thức
  if (select_time) {
    tft.fillRect(46, 130, 15, 6, ILI9341_YELLOW);
    tft.fillRect(100, 130, 15, 6, ILI9341_BLACK);
  } else {
    tft.fillRect(100, 130, 15, 6, ILI9341_YELLOW);
    tft.fillRect(46, 130, 15, 6, ILI9341_BLACK);
  }

  if (bao_thuc)
    tft.drawRGBBitmap(262, 6, image_alarm_icon_pixels, 15, 16);
  else
    tft.fillRect(262, 6, 15, 16, ILI9341_BLACK);

  // Kích hoạt báo thức
  if (gio == gio_baothuc && phut == phut_baothuc && bao_thuc) {
    playbeep();
  }

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setFont(&FreeMono12pt7b);
  tft.setCursor(46, 160);
  if (gio_baothuc < 10) tft.print("0");
  tft.print(gio_baothuc); tft.print(":");
  if (phut_baothuc < 10) tft.print("0");
  tft.print(phut_baothuc);

  delay(1);
}

// ================== HÀM HIỂN THỊ ==================
void creat_srceen(int gio, int phut, int giay, int ngay, float h, float t) {
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setFont();
  tft.setCursor(36, 5);
  tft.println("Ho Chi Minh City");


  tft.fillRoundRect(10, 30, 300, 90, 10, ILI9341_DARKGREY);
  tft.setTextColor(ILI9341_CYAN);
  tft.setFont(&FreeMonoBold24pt7b);
  tft.setTextSize(1);
  tft.setCursor(40, 100);
  if (gio < 10) tft.print("0");
  tft.print(gio); tft.print(":");
  if (phut < 10) tft.print("0");
  tft.print(phut); tft.print(":");
  if (giay < 10) tft.print("0");
  tft.print(giay);

  tft.fillRoundRect(10, 140, 143, 83, 10, ILI9341_DARKGREY);
  tft.setTextColor(ILI9341_BLUE);
  tft.setFont(&FreeMono12pt7b);
  tft.setCursor(46, 219); tft.println(h, 1);
  tft.setCursor(125, 219); tft.print("%");

  tft.setTextColor(ILI9341_RED);
  tft.setCursor(46, 189); tft.println(t, 1);
  tft.setCursor(125, 189); tft.print("*C");

  tft.drawRGBBitmap(20, 145, image_alarm_icon_pixels, 15, 16);
  tft.drawRGBBitmap(7, 4, image_location_pixels, 20, 20);
  tft.drawRGBBitmap(20, 170, image_icon_temp_pixels, 15, 22);
  tft.drawRGBBitmap(20, 200, image_icon_hump_pixels, 15, 22);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&FreeMono12pt7b);
  tft.setCursor(200, 150);
  tft.println("weather:");
}

// ================== CẬP NHẬT ICON WIFI ==================
void updateWiFiIcon() {
  if (WiFi.status() == WL_CONNECTED)
    tft.drawRGBBitmap(285, 6, image_wifi_icon_pixels, 19, 16);
  else
    tft.fillRect(285, 6, 19, 16, ILI9341_BLACK);
}

// ================== ICON THỜI TIẾT ==================
void iconthoitiet(int code, int gio) {
  switch (code) {
    case 200: case 201: case 202: // Thunderstorm with rain
      tft.drawRGBBitmap(215, 163, image_thunder_pixels, 72, 72);
      break;
    case 230: case 231: case 232://Thunderstorm with drizzle
    case 500: case 501: case 502: //Light/Heavy Rain
      tft.drawRGBBitmap(215, 163, image_rain_day_pixels, 72, 72);
      break;
    case 800: // Clear sky
    case 801: case 802: case 803: case 804:
    // Few clouds
      if (gio < 18 && gio > 5)
        tft.drawRGBBitmap(215, 163, image_clear_day_pixels, 72, 72);
      else
        tft.drawRGBBitmap(215, 163, image_clear_night_pixels, 72, 72);
      break;
    
    default:
      tft.drawRGBBitmap(215, 163, image_cloudy_pixels, 72, 72);
      break;
  }
}

// ================== BUZZER ==================
void playbeep() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}
