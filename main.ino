#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <certificates.h>

#define ledR 14
#define ledG 12
#define ServoPin 17

Servo myServo;

String ids[100]; 


// AWS
const char* endpoint = "avmr86pgqcbi1-ats.iot.us-east-2.amazonaws.com";
const char* awsTopic = "sabara/entradas";

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

WiFiClient espClient; // para mqtt
PubSubClient mqttClient(espClient);

WiFiClientSecure net;  // para AWS
PubSubClient awsClient(net);

void conectarWifi(){
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Wi-Fi conectado!");
}

// função para conectar ao MQTT
void connectarMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (mqttClient.connect("ESP32Client-1")) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("Falha na conexão.");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void conectarAWS(){
 // credenciais aws
  net.setCACert(aws_cert_ca);
  net.setCertificate(aws_cert_crt);
  net.setPrivateKey(aws_cert_private);

  awsClient.setServer(endpoint, 8883);
 
 
  Serial.println("Conectando ao AWS IOT");
 
  while (!awsClient.connected()) {
    Serial.print("...");
    if (awsClient.connect("ESP32ClientAWS")) {
      Serial.println("Conectado ao AWS IOT!");
    } else {
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

  conectarWifi();
  connectarMQTT();
  conectarAWS();

  Serial.println("Aproxime um cartão RFID ou digite um ID no Serial: ");
}


void loop() {
  String rfidTag = "";

  // Leitura via cartão RFID real
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) rfidTag += "0";
      rfidTag += String(rfid.uid.uidByte[i], HEX);
    }
    rfidTag.toUpperCase();

    Serial.print("Cartão RFID lido: ");
    Serial.println(rfidTag);

    rfid.PICC_HaltA();         // Finaliza comunicação com o cartão
    rfid.PCD_StopCrypto1();    // Finaliza criptografia
  }

  // Leitura via Serial
  if (Serial.available() > 0) {
    rfidTag = Serial.readStringUntil('\n'); 
    rfidTag.trim();  // remover espaços

    Serial.print("Tag RFID digitada: ");
    Serial.println(rfidTag);
  }

  // Se rfidTag não estiver vazia, processa os dados
  if (rfidTag != "") {
    dateTime = getDateTime();  // atualizar data e hora

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
          ids[i] = "";  // Limpa o ID da lista
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

    if (!mqttClient.connected()) {
      connectarMQTT();
    }
    mqttClient.loop();

    if (!awsClient.connected()) {
      conectarAWS();
    }
    awsClient.loop();
    
    if (mqttClient.publish(mqttTopic, jsonString.c_str())) {
      Serial.println("Dados publicados com sucesso no MQTT!");
    } else {
      Serial.println("Falha ao publicar os dados no MQTT.");
    }

    if (awsClient.publish(awsTopic, jsonString.c_str())) {
      Serial.println("Dados publicados com sucesso na AWS!");
    } else {
      Serial.println("Falha ao publicar os dados na AWS.");
    }
  }

  delay(500); 
}
