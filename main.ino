#include <MFRC522.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define ledR 14
#define ledG 12
#define ServoPin 17

Servo myServo;

String ids[100]; 


// configurações para conectar no wifi
const char* ssid = "nome_wifi";  
const char* password = "senha_wifi"; 

// configuração do para obter data e hora
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // horário de brasília
const int daylightOffset_sec = 0;

// configurando mqtt
const char* mqttServer = "broker.hivemq.com"; 
const int mqttPort = 1883;
const char* mqttTopic = "sabara/entradas";

WiFiClient espClient;
PubSubClient client(espClient);

// função para conectar ao MQTT
void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado com sucesso ao MQTT!!");
    } else {
      Serial.print("Falha na conexão.");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

// variáveis do rfid
#define SS_PIN 23
#define RST_PIN 18
MFRC522 rfid(SS_PIN, RST_PIN);

String dateTime; 

String getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro ao obter hora";
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

bool esta_dentro(String rfidTag) {
  for (int i = 0; i < 100; i++) {
    if (ids[i] == rfidTag) {
      return true;  
    }
  }
  return false;  
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  myServo.attach(ServoPin, 500, 2400);

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov", "pool.ntp.org");

  // conectar no wifi
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Wi-Fi conectado!");

  // conectar no MQTT
  client.setServer(mqttServer, mqttPort);
  connectToMQTT();

  Serial.println("Aproxime um cartão RFID ou digite um ID no Serial: ");
}

void loop() {
  // código para simular entrada de um cartão/pulseira através da Serial
  if (Serial.available() > 0) {
    String rfidTag = Serial.readStringUntil('\n'); 
    rfidTag.trim();  // remover espaços

    Serial.print("Tag RFID detectada: ");
    Serial.println(rfidTag);

    dateTime = getDateTime();  // atualizar data e hora

    // verificar se o paciente está dentro ou não
    String message = "";
    if (!esta_dentro(rfidTag)) {
      Serial.println("Entrada registrada.");
      digitalWrite(ledG, HIGH); 
      digitalWrite(ledR, LOW); 
      myServo.write(0);
      message = "Entrada: " + rfidTag + " em " + dateTime;
      Serial.println(message);

        for (int i = 0; i < 100; i++) {
          if (ids[i] == "") {  
            ids[i] = rfidTag;
            break;
          }
      }
    } else {
      Serial.println("Saída registrada.");
      digitalWrite(ledG, LOW); 
      digitalWrite(ledR, HIGH); 
      myServo.write(90);
      message = "Saída: " + rfidTag + " em " + dateTime;
      Serial.println(message);

      for (int i = 0; i < 100; i++) {
        if (ids[i] == rfidTag) {
          ids[i] = ""; 
          break;
        }
      }

    }

    
    StaticJsonDocument<200> json;
    json["ID"] = rfidTag;
    json["Hora"] = dateTime;
    json["status"] = esta_dentro(rfidTag) ? "Entrada" : "Saída";

    
    String jsonString;
    serializeJson(json, jsonString);

    if (!client.connected()) {
      connectToMQTT();
    }
    client.loop();
    
    if (client.publish(mqttTopic, jsonString.c_str())) {
    Serial.println("Dados publicados com sucesso!");
  } else {
    Serial.println("Falha ao publicar os dados.");
  }

  }  
  delay(500); 
}
