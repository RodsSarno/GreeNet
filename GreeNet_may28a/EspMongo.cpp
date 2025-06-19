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

    WiFiClientSecure client;
    client.setInsecure(); // Para HTTPS com certificado autoassinado (Vercel)

    HTTPClient http;
    http.setTimeout(15000); // 15 segundos
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Serial.println("Iniciando requisição para: " + _url);

    if (!http.begin(client, _url)) {
        Serial.println("Connection failed: begin() retornou false");
        return false;
    }

    // ⚠️ Agora que a conexão foi iniciada, podemos configurar os headers
    http.useHTTP10(true); // Ajuda com servidores que não suportam chunked
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP8266");

    String json = getDataJSON();
    Serial.println("Payload JSON: " + json);

    int httpCode = http.POST(json);

    Serial.print("Código HTTP: ");
    Serial.println(httpCode);

    if (httpCode >= 200 && httpCode < 300) {
        String response = http.getString();
        Serial.println("Resposta: " + response);
        http.end();
        return true;
    } else {
        Serial.println("POST falhou");
        if (httpCode > 0) {
            Serial.print("Resposta do servidor: ");
            Serial.println(http.getString());
        } else {
            Serial.print("Erro baixo nível: ");
            Serial.println(http.errorToString(httpCode));
        }
        http.end();
        return false;
    }
}
