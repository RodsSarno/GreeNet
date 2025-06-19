#include <ESP8266WiFi.h> // Adicione esta linha
#include "EspMongo.h"

EspMongo::EspMongo(const String& url) {
    this->setUrl(url);
}

void EspMongo::setUrl(const String& url) {
    _url = url;
}

void EspMongo::setAirQuality(float air_quality) {
    _air_quality = air_quality;
}

void EspMongo::setTemperature(float temperature) {
    _temperature = temperature;
}

void EspMongo::setRelativeHumidity(float relative_humidity) {
    _relative_humidity = relative_humidity;
}

void EspMongo::setSoilHumidity(float soil_humidity) {
    _soil_humidity = soil_humidity;
}

void EspMongo::setLightIntensity(float light_intensity) {
    _light_intensity = light_intensity;
}

String EspMongo::getDataJSON() {
    StaticJsonDocument<256> doc;
    doc["air_quality"] = _air_quality;
    doc["temperature"] = _temperature;
    doc["relative_humidity"] = _relative_humidity;
    doc["soil_humidity"] = _soil_humidity;
    doc["light_intensity"] = _light_intensity;

    String json;
    serializeJson(doc, json);
    return json;
}

bool EspMongo::sendJsonData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }

  WiFiClient client;
  HTTPClient http;
  
  // Configura redirecionamentos
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);  // Aumenta timeout para 15s

  if (!http.begin(client, _url)) {
    Serial.println("Connection failed");
    return false;
  }
  
  http.addHeader("Content-Type", "application/json");
  
  String json = getDataJSON();
  int httpCode = http.POST(json);

  // Trata códigos de sucesso (2xx)
  if (httpCode >= 200 && httpCode < 300) {
    String response = http.getString();
    Serial.println("Response: " + response);
    http.end();
    return true;
  } else {
    Serial.print("POST failed, error: ");
    Serial.println(httpCode);
    
    // Mostra detalhes do erro
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    }
    http.end();
    return false;
  }
}