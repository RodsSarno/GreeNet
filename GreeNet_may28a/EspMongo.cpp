#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> // Necessário para HTTPS
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

    // Usa HTTPS com verificação de certificado desativada
    WiFiClientSecure client;
    client.setFingerprint("E3:1E:98:A9:DD:8B:60:D7:46:D6:CC:B1:15:28:72:F4:76:3D:CE:C1");


    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000); // 15 segundos de timeout

    Serial.println("Iniciando requisição para: " + _url);

    if (!http.begin(client, _url)) {
        Serial.println("Connection failed: begin() retornou false");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    String json = getDataJSON();
    Serial.println("Payload JSON: " + json);

    int httpCode = http.POST(json);

    if (httpCode >= 200 && httpCode < 300) {
        String response = http.getString();
        Serial.println("Resposta: " + response);
        http.end();
        return true;
    } else {
        Serial.print("POST falhou, código de erro: ");
        Serial.println(httpCode);

        if (httpCode > 0) {
            String response = http.getString();
            Serial.println("Resposta do servidor: " + response);
        }

        http.end();
        return false;
    }
}