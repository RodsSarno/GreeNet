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

    HTTPClient http;
    http.begin(_url);
    http.addHeader("Content-Type", "application/json");

    String json = getData();
    int httpCode = http.POST(json);

    if (httpCode == 200) {
        String response = http.getString();
        Serial.println("Response: " + response);
        http.end();
        return true;
    } else {
        Serial.print("POST failed, error: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }
}
