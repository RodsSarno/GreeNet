// Definições Blynk DEVEM vir ANTES das includes
#define BLYNK_TEMPLATE_ID "TMPL26N2VobHl"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "VHP5hX3dU-zQ_0f66P2JHZqXjuY7t1Fl"
#define BLYNK_PRINT Serial

#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "ATH21.h"
#include "ENS160.h"
#include <BlynkSimpleEsp8266.h>

// Credenciais WiFi
char ssid[] = "Salada de Bacon";
char pass[] = "persona345";

// Endpoint do backend
const char* serverUrl = "https://pi-iot-backend.vercel.app/api/measure";

// Pinos dos sensores
#define SOIL_MOISTURE_PIN 0
#define LIGHT_SENSOR_PIN A0

// Endereços I2C
#define ATH21_ADDRESS 0x38
#define ENS160_ADDRESS 0x53

BlynkTimer timer;

// Variáveis globais dos sensores
float temperature = 0.0;
float umidade = 0.0;
float LUX = 0.0;
int AQI = 0;
bool SoilH = false;

// Função para ler sensores e atualizar variáveis
void readSensors() {
  float ath_temp, ath_rh;
  ATH21_read(&ath_temp, &ath_rh);
  if (ath_temp >= -40.0 && ath_temp <= 85.0 && ath_rh >= 0.0 && ath_rh <= 100.0) {
    temperature = ath_temp;
    umidade = ath_rh;
    ENS160_write_temp_in(ath_temp);
    ENS160_write_rh_in(ath_rh);
  }
  AQI = ENS160_read_AQI();
  int rawLight = analogRead(LIGHT_SENSOR_PIN);
  float voltage = rawLight * (3.3 / 1023.0);
  LUX = voltage * 500.0;
  SoilH = (digitalRead(SOIL_MOISTURE_PIN) == LOW);
}

// Função para enviar dados ao backend
void sendSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    if (https.begin(client, serverUrl)) {
      https.addHeader("Content-Type", "application/json");
      String payload = "{";
      payload += "\"air_quality\": " + String(AQI) + ",";
      payload += "\"temperature\": " + String(temperature, 2) + ",";
      payload += "\"relative_humidity\": " + String(umidade, 2) + ",";
      payload += "\"soil_humidity\": " + String(SoilH ? 100 : 0) + ",";
      payload += "\"light_intensity\": " + String(int(LUX));
      payload += "}";
      int httpCode = https.POST(payload);
      if (httpCode > 0) {
        Serial.println("HTTPS Response code: " + String(httpCode));
        String response = https.getString();
        Serial.println("Response: " + response);
      } else {
        Serial.println("POST failed, error: " + String(httpCode));
      }
      https.end();
    } else {
      Serial.println("Unable to connect to server!");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}

// Função para enviar dados para o Blynk
void sendToBlynk() {
  Blynk.virtualWrite(V4, temperature);         // V4: Temperatura
  Blynk.virtualWrite(V5, umidade);             // V5: Umidade
  Blynk.virtualWrite(V6, AQI);                 // V6: Qualidade do ar
  Blynk.virtualWrite(V7, SoilH ? 100 : 0);     // V7: Umidade do solo (%)
  Blynk.virtualWrite(V8, int(LUX));
  Serial.println("Dados enviados ao Blynk");
}

void setup() {
  Serial.begin(115200);
  
  // Inicialização I2C
  Wire.begin(4, 5); // SDA=D2(4), SCL=D1(5)
  delay(100);
  
  // Inicialização sensores
  ENS160_init();
  pinMode(SOIL_MOISTURE_PIN, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // Conexão WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicialização Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk inicializado");

  // Configura timer para executar a cada 10 segundos
  timer.setInterval(10000L, []() {
    readSensors();
    sendToBlynk();
    sendSensorData();
  });
}

void loop() {
  Blynk.run();
  timer.run();
}