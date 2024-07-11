//file: main.cpp
#include "Arduino.h"
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_PCF8574.h>
#include <TinyGPS++.h>

// Замените на название вашей сети (SSID) и пароль
const char* ssid = "";
const char* password = "";

// Определяем пины для RX и TX
#define RX_PIN 20
#define TX_PIN 21

// Инициализация дисплея с адресом 0x27 для 16x2 дисплея
LiquidCrystal_PCF8574 lcd(0x27);

// Инициализация GPS-модуля
TinyGPSPlus gps;

// Адрес датчика HDC1080
#define HDC1080_ADDR 0x40

WebServer server(80);

// Функция для получения температуры
float readTemperature() {
    unsigned int data[2];
    Wire.beginTransmission(HDC1080_ADDR);
    Wire.write(0x00); // Адрес регистра температуры
    Wire.endTransmission();
    delay(5); // Ожидание завершения измерения
    Wire.requestFrom(HDC1080_ADDR, 2);
    if (Wire.available() == 2) {
        data[0] = Wire.read();
        data[1] = Wire.read();
        unsigned int temp_raw = (data[0] << 8) | data[1];
        float temperature = (temp_raw * 165.0 / 65536.0) - 40.0;
        return temperature;
    } else {
        return 0.0;
    }
}

// Функция для получения влажности
float readHumidity() {
    unsigned int data[2];
    Wire.beginTransmission(HDC1080_ADDR);
    Wire.write(0x01); // Адрес регистра влажности
    Wire.endTransmission();
    delay(5); // Ожидание завершения измерения
    Wire.requestFrom(HDC1080_ADDR, 2);
    if (Wire.available() == 2) {
        data[0] = Wire.read();
        data[1] = Wire.read();
        unsigned int humidity_raw = (data[0] << 8) | data[1];
        float humidity = (humidity_raw * 100.0 / 65536.0);
        return humidity;
    } else {
        return 0.0;
    }
}

// Обработчик для корневого URL "/"
void handleRoot() {
    float temperature = readTemperature();
    float humidity = readHumidity();

    String html = "<!DOCTYPE html><html lang='ru'><head>";
    html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>**ТЕМПВЛАЖ**</title></head><body style='font-family: Arial, sans-serif; line-height: 1.6; color: #333; margin: 0; padding: 20px; background-color: #7e8aa3;'>";
    html += "<header style='background-color: #4567c3; color: white; padding: 20px 0; text-align: center;'>";
    html += "<h1>Добро пожаловать на страницу измерения темп и влаж</h1><p style='margin: 0;'>использование ESP32 и модуля HDC1080 для измерения температуры и влажности, а также отображение на веб-странице в реальном времени </p></header>";
    html += "<nav style='background-color: #333; color: white; padding: 10px 20px;'><ul style='list-style-type: none; margin: 0; padding: 0; display: flex; justify-content: space-around;'>";
    html += "<li><a href='/data' style='color: white; text-decoration: none;'>JSON ДАННЫЕ</a></li><li><a href='/data' style='color: white; text-decoration: none;'>JSON ДАННЫЕ</a></li><li><a href='/data' style='color: white; text-decoration: none;'>JSON ДАННЫЕ</a></li><li><a href='/data' style='color: white; text-decoration: none;'>JSON ДАННЫЕ</a></li></ul></nav>";
    html += "<main style='padding: 20px; background-color: white; margin-top: 20px; box-shadow: 0 0 10px rgba(3 4 255);'>";
    html += "<section><h2>О чем?</h2><p>Разработка веб-страницы с отображением в реальном времени температуры и влажности на основе датичка HDC1080 с использованием микроконтроллера <a href='https://www.wemos.cc/en/latest/c3/c3_mini.html' style='color: blue; text-decoration: underline dotted;'>WEMOS LOLIN C3 MINI(на базе ESP32) </a></p>";
    html += "<img src='https://i.gifer.com/Vp3L.gif' alt='попытка вставить норм изображение :p ' style='width: 10%; height: auto; margin-top: 10px;'></section>";
    html += "<section style='margin-top: 20px;'><h2>как это рабоатет?</h2><ul style='list-style-type: disc; padding-left: 20px;'><li>подключили и идентифицировали ESP32 к пк через Arduino IDE</li><li>присоединили к плате датчик температуры и влажности HDC1080</li><li>с помощью оссобености платы смогли открыть сервер на к-ом сейчас этот сайт и хостится</li></ul></section>";
    html += "<section style='margin-top: 20px;'><h2>связи</h2><table style='width: 100%; border-collapse: collapse; margin-top: 10px;'><tr><th style='border: 1px solid #ddd; padding: 8px; background-color: #f2f2f2;'>адрес</th><td style='border: 1px solid #ddd; padding: 8px;'>Новоизмайловский пр., 46, к. 2, Санкт-Петербург</td></tr>";
    html += "<tr><th style='border: 1px solid #ddd; padding: 8px; background-color: #f2f2f2;'>телефон</th><td style='border: 1px solid #ddd; padding: 8px;'>+7 812 374-21-46</td></tr>";
    html += "<tr><th style='border: 1px solid #ddd; padding: 8px; background-color: #f2f2f2;'>еmail</th><td style='border: 1px solid #ddd; padding: 8px;'>lenelektro@mail.ru</td></tr></table></section>";
    html += "<section style='margin-top: 20px;'><h2>текущие данные по датчику</h2>";
    html += "<div style='text-align: center;'>";
    html += "<img src='https://flomaster.top/uploads/posts/2023-01/1673425143_flomaster-club-p-risunok-temperatura-vkontakte-2.jpg' alt='Иконка температуры' style='width: 10%; height: auto; margin-top: 10px;'>";
    html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Температура: <span id='temperature'>" + String(temperature) + "</span> °C</p></div>";
    html += "<div style='text-align: center;'>";
    html += "<img src='https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQYd0rP_0V3gWgAaPxX1sf2y3h7DBDwS2f31aIbFBH5wSPpc_uUB1CiKlf0NdV5AQ_ZxCM&usqp=CAU' alt='Иконка влажности' style='width: 10%; height: auto; margin-top: 10px;'>";
    html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Влажность: <span id='humidity'>" + String(humidity) + "</span> %</p></div>";
    
    if (gps.location.isValid()) {
        float lat = gps.location.lat();
        float lng = gps.location.lng();
        float alt = gps.altitude.meters();
        float speed = gps.speed.kmph();
        int sat = gps.satellites.value();
        html += "<div style='text-align: center;'>";
        html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Широта: <span id='latitude'>" + String(lat, 6) + "</span></p></div>";
        html += "<div style='text-align: center;'>";
        html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Долгота: <span id='longitude'>" + String(lng, 6) + "</span></p></div>";
        html += "<div style='text-align: center;'>";
        html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Высота: <span id='altitude'>" + String(alt, 2) + "</span> м</p></div>";
        html += "<div style='text-align: center;'>";
        html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Скорость: <span id='speed'>" + String(speed, 2) + "</span> км/ч</p></div>";
        html += "<div style='text-align: center;'>";
        html += "<p style='display: inline-block; font-size: 1.5em; font-weight: bold; vertical-align: middle;'>Спутники: <span id='satellites'>" + String(sat) + "</span></p></div>";
    }
    
    html += "</main><footer style='background-color: #333; color: white; text-align: center; padding: 10px; margin-top: 20px;'><p>&copy; 2024 Специально для АО Ленэлектро. Все права защищены.</p></footer>";
    html += "<script>function updateData() { fetch('/data').then(response => response.json()).then(data => { document.getElementById('temperature').innerText = data.temperature; document.getElementById('humidity').innerText = data.humidity; document.getElementById('latitude').innerText = data.latitude; document.getElementById('longitude').innerText = data.longitude; document.getElementById('altitude').innerText = data.altitude; document.getElementById('speed').innerText = data.speed; document.getElementById('satellites').innerText = data.satellites; }); } setInterval(updateData, 2000);</script>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

// Обработчик для URL "/data"
void handleData() {
    float temperature = readTemperature();
    float humidity = readHumidity();
    float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
    float lng = gps.location.isValid() ? gps.location.lng() : 0.0;
    float alt = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    float speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
    int sat = gps.satellites.isValid() ? gps.satellites.value() : 0;

    String json = "{";
    json += "\"temperature\": " + String(temperature) + ",";
    json += "\"humidity\": " + String(humidity) + ",";
    json += "\"latitude\": " + String(lat, 6) + ",";
    json += "\"longitude\": " + String(lng, 6) + ",";
    json += "\"altitude\": " + String(alt, 2) + ",";
    json += "\"speed\": " + String(speed, 2) + ",";
    json += "\"satellites\": " + String(sat);
    json += "}";

    server.send(200, "application/json", json);
}



extern "C" void app_main()
{
  initArduino();

 
    Serial.begin(115200);
    Wire.begin();

    // Инициализация дисплея
    lcd.begin(16, 2);
    lcd.setBacklight(255);

    // Инициализация WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Подключение к WiFi...");
    }
    Serial.println("WiFi подключен!");
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());

    // Настройка веб-сервера
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("HTTP сервер запущен");

    // Настройка GPS
    Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);



  // Arduino-like loop()
while (true) {
    // Обновление GPS данных
    while (Serial1.available() > 0) {
        gps.encode(Serial1.read());
    }
    while (Serial1.available() > 0) {
        char c = Serial1.read();
        gps.encode(c);
    }
    // Обновление дисплея
    if (gps.time.isValid() && gps.date.isValid()) {
        // Получаем текущее время и дату
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
        int day = gps.date.day();
        int month = gps.date.month();
        int year = gps.date.year();
        float temperature = readTemperature();
        float humidity = readHumidity();
        // Получаем данные GPS
        float lat = gps.location.lat();
        float lng = gps.location.lng();
        float alt = gps.altitude.meters();
        float speed = gps.speed.kmph();
        int sat = gps.satellites.value();

        // Обновляем дисплей
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        lcd.print(hour + 4);
        lcd.print(":");
        lcd.print(minute);
        lcd.print(":");
        lcd.print(second);

        lcd.setCursor(0, 1);
        lcd.print("Date: ");
        lcd.print(day);
        lcd.print("/");
        lcd.print(month);
        lcd.print("/");
        lcd.print(year);

        delay(1000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Latit: ");
        lcd.print(lat, 6);

        lcd.setCursor(0, 1);
        lcd.print("Longi: ");
        lcd.print(lng, 6);

        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Altit: ");
        lcd.print(alt);

        lcd.setCursor(0, 1);
        lcd.print("Speed: ");
        lcd.print(speed);
        lcd.print("km/h");

        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Satellites: ");
        lcd.print(sat);

        lcd.setCursor(0, 1);
        lcd.print("*-*-*-*-*-*-*-*-*-*-*-*");

        delay(1000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp:  ");
        lcd.print(temperature);
        lcd.print(" C");


        lcd.setCursor(0, 1);
        lcd.print("Hum:   ");
        lcd.print(humidity);
        lcd.print(" %");

        delay(1000);

        
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No GPS data");
        lcd.setCursor(0, 1);
        lcd.print("received yet.");
    }
    // Обработка HTTP запросов
    server.handleClient();
}


  // WARNING: if program reaches end of function app_main() the MCU will restart.
}