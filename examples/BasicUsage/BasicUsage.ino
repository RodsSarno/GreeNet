#include <ESP8266WiFi.h>
#include "EspMongo.h"

const char* ssid = "your-ssid";               // Replace with your Wi-Fi SSID
const char* password = "your-password";       // Replace with your Wi-Fi password

const char* backendUrl = "https://pi-iot-backend.vercel.app/api/measure";  // Replace with your backend URL

EspMongo mongo(backendUrl);

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void simulateSensorData() {
  float airQuality = random(300, 800) / 10.0;          // e.g. 30.0 - 80.0
  float temperature = random(200, 350) / 10.0;         // e.g. 20.0 - 35.0
  float relativeHumidity = random(300, 800) / 10.0;    // e.g. 30.0 - 80.0
  float soilHumidity = random(200, 700) / 10.0;        // e.g. 20.0 - 70.0
  float lightIntensity = random(100, 1000);            // e.g. 100 - 1000 lux

  // Set values in the library
  mongo.setAirQuality(airQuality);
  mongo.setTemperature(temperature);
  mongo.setRelativeHumidity(relativeHumidity);
  mongo.setSoilHumidity(soilHumidity);
  mongo.setLightIntensity(lightIntensity);

  Serial.println("Simulated sensor values:");
  Serial.println(mongo.getData());
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Let serial stabilize

  connectToWiFi();
  simulateSensorData();

  if (mongo.sendJsonData()) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Failed to send data.");
  }
}

void loop() {
  // Optional: send data periodically
  delay(60000);  // Wait 1 minute before next send
}
