#define SOIL_DEBOUNCE_TIME 500

#include <ESP8266WiFi.h>  
#include <ESP8266HTTPClient.h>
#include "arduino_secrets.h"
#include "thingProperties.h"
#include <Wire.h>
#include "ATH21.h"
#include "ENS160.h"
#include "EspMongo.h"

// Configuração dos pinos
#define SOIL_MOISTURE_PIN 0      // Pino digital para sensor de umidade do solo (D3 = GPIO0)
#define LIGHT_SENSOR_PIN A0      // Pino analógico para TEMT6000 (luminosidade)

// Endereços I2C
#define ATH21_ADDRESS 0x38
#define ENS160_ADDRESS 0x53

// Variáveis para leitura dos sensores
float ath_temp, ath_rh;
uint16_t tvoc, eco2;
uint8_t aqi_UEB;

// Controle de intervalo de leitura
unsigned long lastMillis = 0;
const long interval = 10000;  // Intervalo de 10 segundos

// Variáveis para debounce do solo
bool lastSoilState = false;
unsigned long lastSoilRead = 0;

// Controle para envio ao backend
unsigned long lastBackendMillis = 0;
const long backendInterval = 60000; // 60 segundos
EspMongo mongo(SECRET_BACKEND_URL);

// Protótipos de função
bool testI2CDevice(byte address, const char* deviceName);
void readSensors();
void printSensorData();
void sendToBackend();

void setup() {
  // Inicialização serial com tratamento robusto
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera até a porta serial estar pronta
  }
  delay(2000);  // Espera adicional para estabilização
  
  Serial.println("\n\n=== Sistema GreeNet - Inicialização ===");
  Serial.println("Versão: 1.0");
  Serial.println("Baud rate: 115200");
  Serial.println("Verificando componentes...");

  // Inicialização I2C
  Wire.begin(4, 5); // SDA=D2(4), SCL=D1(5)
  delay(100);
  
  // Teste de comunicação com ENS160
  bool ens160_ok = testI2CDevice(ENS160_ADDRESS, "ENS160 (Qualidade do Ar)");
  if (ens160_ok) {
    ENS160_init();
    Serial.println("ENS160 configurado no modo Gas Sensing");
  }

  // Teste de comunicação com ATH21
  testI2CDevice(ATH21_ADDRESS, "ATH21 (Temp/Umidade)");

  // Configuração de pinos
  pinMode(SOIL_MOISTURE_PIN, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  Serial.println("Pinos configurados:");
  Serial.println("- Solo: D3 (INPUT_PULLUP)");
  Serial.println("- Luz: A0 (INPUT)");

  // Conexão com Arduino IoT Cloud
  Serial.println("\nIniciando conexão com Arduino IoT Cloud...");
  initProperties();
  
  // Configuração para ESP8266
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  
  // Conexão com tratamento de erro
  int cloud_retry = 0;
  while (cloud_retry < 10) {
    if (ArduinoCloud.begin(ArduinoIoTPreferredConnection)) {
      Serial.println("Conexão com a nuvem estabelecida!");
      break;
    }
    delay(2000);
    Serial.print(".");
    cloud_retry++;
  }
  
  if (cloud_retry >= 10) {
    Serial.println("\nERRO: Falha ao conectar com a nuvem!");
  } else {
    Serial.println("Sistema totalmente operacional\n");
  }

  // Mostra URL do backend
  Serial.print("Backend URL: ");
  Serial.println(SECRET_BACKEND_URL);
}

void loop() {
  ArduinoCloud.update();
  
  unsigned long currentMillis = millis();
  
  // Leitura periódica dos sensores
  if (currentMillis - lastMillis >= interval) {
    lastMillis = currentMillis;
    readSensors();
    printSensorData();
  }
  
  // Envio periódico para o backend
  if (currentMillis - lastBackendMillis >= backendInterval) {
    lastBackendMillis = currentMillis;
    sendToBackend();
  }
}

bool testI2CDevice(byte address, const char* deviceName) {
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("- ");
    Serial.print(deviceName);
    Serial.println(": OK");
    return true;
  } else {
    Serial.print("- ");
    Serial.print(deviceName);
    Serial.print(": FALHA (Erro ");
    Serial.print(error);
    Serial.println(")");
    return false;
  }
}

void readSensors() {
  // Leitura ATH21 com verificação de valores
  bool ath21_valid = false;
  for(int i = 0; i < 3; i++) { // Tenta até 3 vezes
    ATH21_read(&ath_temp, &ath_rh);
    
    // Verifica valores fisicamente possíveis
    if(ath_temp >= -40.0 && ath_temp <= 85.0 && 
       ath_rh >= 0.0 && ath_rh <= 100.0) {
      ath21_valid = true;
      break;
    }
    delay(50);

  static bool soilReading = false;
  static unsigned long lastSoilChange = 0;

  bool currentReading = (digitalRead(SOIL_MOISTURE_PIN) == LOW);
    if (currentReading != soilReading) {
      if (millis() - lastSoilChange > SOIL_DEBOUNCE_TIME) {
        SoilH = currentReading;
        soilReading = currentReading;
        lastSoilChange = millis();
      }
    } else {
      lastSoilChange = millis();
    }
  }

  if(ath21_valid) {
    temperature = ath_temp;  // Sincroniza com cloud variable
    umidade = ath_rh;        // Sincroniza com cloud variable

    // Envia dados para compensação do ENS160
    ENS160_write_temp_in(ath_temp);
    ENS160_write_rh_in(ath_rh);
  } else {
    Serial.println("Erro: Valores inválidos do ATH21");
  }

  // Leitura ENS160
  aqi_UEB = ENS160_read_AQI();
  AQI = aqi_UEB;  // Sincroniza com cloud variable

  // Leitura luminosidade (TEMT6000) - Fórmula corrigida
  int rawLight = analogRead(LIGHT_SENSOR_PIN);
  float voltage = rawLight * (3.3 / 1023.0);
  LUX = voltage * 500.0;  // Conversão para lux

  // Leitura umidade do solo com debounce
  unsigned long currentMillisSoil = millis();
  if (currentMillisSoil - lastSoilRead >= 200) { // Debounce de 200ms
    bool currentState = (digitalRead(SOIL_MOISTURE_PIN) == LOW); // LOW = úmido
    
    if (currentState != lastSoilState) {
      SoilH = currentState;  // Atualiza variável da nuvem
      ArduinoCloud.update(); // Força sincronização imediata
      lastSoilState = currentState;
    }
    lastSoilRead = currentMillisSoil;
  }
}

void printSensorData() {
  Serial.println("----- Dados dos Sensores -----");
  Serial.print("Temperatura: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Umidade: "); Serial.print(umidade); Serial.println(" %");
  Serial.print("Luminosidade: "); Serial.print(LUX); Serial.println(" lux");
  Serial.print("AQI: "); Serial.println(AQI);
  Serial.print("Solo: "); Serial.println(SoilH ? "Úmido" : "Seco");
  Serial.println("----------------------------");
}

void sendToBackend() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi offline - pulando envio ao backend");
      return;
  }

  // Prepara e envia dados para o MongoDB
  mongo.setAirQuality(AQI);
  mongo.setTemperature(temperature);
  mongo.setRelativeHumidity(umidade);
  mongo.setSoilHumidity(SoilH ? 100.0 : 0.0); // Converte bool para %
  mongo.setLightIntensity(LUX);

  Serial.println("Enviando dados para MongoDB...");
  
  if (mongo.sendJsonData()) {
    Serial.println("Dados enviados para MongoDB!");
  } else {
    Serial.println("Falha no envio ao MongoDB");
    Serial.print("Status WiFi: ");
    Serial.println(WiFi.status());
  }
}

// Funções de callback
void onTemperatureChange() { /* Executa quando temperatura muda no dashboard */ }
void onUmidadeChange() { /* Executa quando umidade muda no dashboard */ }
void onAQIChange() { /* Executa quando AQI muda no dashboard */ }
void onLUXChange() { /* Executa quando luminosidade muda no dashboard */ }
void onSoilHChange() { /* Executa quando estado do solo muda no dashboard */ }