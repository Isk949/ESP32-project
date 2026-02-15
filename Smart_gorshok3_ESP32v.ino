#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "esp_sleep.h"


#define WIFI_SSID 
#define WIFI_PASSWORD 


#define BOT_TOKEN 
#define CHAT_ID 
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);


#define DHTPIN 4
#define DHTTYPE DHT22
#define PUMP_PIN 16
#define FAN_PIN 17
#define SOIL_PIN 34
#define WATER_PIN 35


DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C LCD(0x27, 16, 2);


#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP  28800  //

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(WATER_PIN, INPUT);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);

  LCD.init();
  LCD.backlight();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilVal = analogRead(SOIL_PIN);
  int waterVal = analogRead(WATER_PIN);

  int soilPerc = map(soilVal, 1000, 3000, 100, 0);
  if (soilPerc < 0) soilPerc = 0;
  if (soilPerc > 100) soilPerc = 100;

  int waterPerc = map(waterVal, 100, 700, 0, 100);
  if (waterPerc < 0) waterPerc = 0;
  if (waterPerc > 100) waterPerc = 100;

  LCD.setCursor(0, 0);
  LCD.print("H:" + String(h) + "% T:" + String(t) + "C");
  LCD.setCursor(0, 1);
  LCD.print("SM:" + String(soilPerc) + "% W:" + String(waterPerc) + "%");


  if (WiFi.status() == WL_CONNECTED) {
    String msg = "🌱 Авто-отчёт:\n";
    msg += "💧 Влажность воздуха: " + String(h) + "%\n";
    msg += "🌡 Температура: " + String(t) + " C\n";
    msg += "🌿 Почва: " + String(soilPerc) + "%\n";
    msg += "🫗 Вода: " + String(waterPerc) + "%";
    bot.sendMessage(CHAT_ID, msg, "");
  }

  // Автополив
  if (soilPerc < 40) {
    digitalWrite(PUMP_PIN, LOW);
    delay(2000); // насос работает 2 сек
    digitalWrite(PUMP_PIN, HIGH);
  }

  // Автовентиляция
  if (t > 28) {
    digitalWrite(FAN_PIN, LOW);
    delay(5000); // кулер 5 сек
    digitalWrite(FAN_PIN, HIGH);
  }

  Serial.println("Уходим в сон на 8 часов...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
}

