#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>

// ---------------- CONFIG ----------------
const char* ssid = "sihgo";
const char* password = "password";
const char* blynkToken = "BLYNK_TOKEN"; //private token
const char* weatherApiKey = "OPENWEATHERMAP_KEY"; //priv key
const char* city = "Bangalore";

const int batteryPin = A0;
const int solarPin = A0;   
const float batteryLowThreshold = 3.3;
const int backupPin = D1;  

const unsigned long interval = 60000; // 1 min interval
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode(backupPin, OUTPUT);
  digitalWrite(backupPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  Blynk.begin(blynkToken, ssid, password);
}

float readVoltage(int pin) {
  int analogValue = analogRead(pin);
  float voltage = analogValue * (5.0 / 1023.0) * 2;
  return voltage;
}


String getWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&appid=" + String(weatherApiKey);
    http.begin(url);
    int httpCode = http.GET();
    String payload = "{}";
    if (httpCode > 0) {
      payload = http.getString();
    }
    http.end();
    return payload;
  }
  return "{}";
}


bool isCloudy(String payload) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) return false;
  int cloud = doc["clouds"]["all"]; // 0-100
  return cloud > 50;
}


void sendData(float batteryV, float solarV) {
  Blynk.virtualWrite(V1, batteryV);
  Blynk.virtualWrite(V2, solarV);
}


void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float batteryV = readVoltage(batteryPin);
    float solarV = readVoltage(solarPin);

    // Send battery and solar data
    sendData(batteryV, solarV);

    // Check weather forecast
    String weatherData = getWeather();
    if (isCloudy(weatherData)) {
      Blynk.notify("Cloudy forecast! Optimize your power usage.");
    }

    // Check battery low
    if (batteryV < batteryLowThreshold && solarV < 4.0) { //low charge + low solar ouput
      digitalWrite(backupPin, HIGH); // Switch to backup
      Blynk.notify("Battery low! Switched to reserve battery.");
    } else {
      digitalWrite(backupPin, LOW);
    }

    Serial.print("Battery: "); Serial.print(batteryV); Serial.print("V, ");
    Serial.print("Solar: "); Serial.print(solarV); Serial.println("V");
  }
}
