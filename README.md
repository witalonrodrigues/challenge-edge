# Mapa da Dor - Sistema de Registro de Entrada e Saída (RFID + MQTT)
Este projeto foi desenvolvido como parte de uma solução para o **Hospital Infantil Sabará**, com o objetivo de monitorar a movimentação de pacientes nas alas hospitalares através de um sensor RFID. Os dados são enviados via protocolo MQTT para um painel de visualização em tempo real.

---

## Funcionalidades

- Leitura de cartões/pulseiras RFID com ESP32  
- Registro automático de entrada e saída de pacientes  
- Envio de dados (ID, horário e status) via MQTT  
- Visualização em tempo real com apps como MyMQTT, MQTT Explorer ou dashboards MQTT

## Tecnologias utilizadas

- **ESP32**  
- **MFRC522** (leitor RFID)  
- **ArduinoJson** (formatação dos dados em JSON)  
- **PubSubClient** (envio MQTT)  
- **Wi-Fi** (Wokwi-GUEST ou rede local)  
- **Broker MQTT público**: [HiveMQ](https://broker.hivemq.com)  
- **Dashboard**: MyMQTT app (Android) ou HiveMQ Web Client

---
## Como Funciona

1. O paciente aproxima a pulseira/cartão RFID do leitor (nesse caso foi utilizado simulação de entrada de um ID atráves da Serial para simulação no wokwi).

2. O sistema identifica se é uma entrada ou saída:

3. Se o ID não está registrado → Entrada

4. Se o ID já está registrado → Saída

5. Um JSON com os dados é publicado no tópico sabara/entradas.

6. O dashboard conectado exibe em tempo real.

## Estrutura da Mensagem MQTT

As mensagens publicadas no tópico MQTT são no formato JSON:

```json
{
  "ID": "AB12CD34",
  "Hora": "12/04/2025 15:32:10",
  "status": "Entrada"
}

```
## Configurações

REDE WI-FI

```
const char* ssid = "Wokwi-GUEST";
const char* password = "";
```

BROKER MQTT

```
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttTopic = "sabara/entradas";
```

## Visualização com MyMQTT
1. Abra o app MyMQTT

2. Conecte com:

Host: broker.hivemq.com

Port: 1883

3. Vá até a aba Subscribe

4. Insira o tópico: sabara/entradas

5. Clique em Subscribe

As mensagens devem aparecer automaticamente a cada registro de entrada ou saída.

## Autores

RM5599023 - WITALON ANTONIO RODRIGUES | 1ESPB
