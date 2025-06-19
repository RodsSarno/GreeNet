#ifndef EspMongo_H
#define EspMongo_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

// Usaremos HTTPClient genérico que funciona tanto para ESP8266 quanto ESP32
#include <ESP8266HTTPClient.h>

class EspMongo {
public:
    EspMongo(const String& url);
    void setUrl(const String& url);
    void setAirQuality(float air_quality);
    void setTemperature(float temperature);
    void setRelativeHumidity(float relative_humidity);
    void setSoilHumidity(float soil_humidity);
    void setLightIntensity(float light_intensity);
    String getDataJSON();
    bool sendJsonData();

private:
    String _url;
    float _air_quality;
    float _temperature;
    float _relative_humidity;
    float _soil_humidity;
    float _light_intensity;
};

#endif