# Mapa da Dor - Sistema de Registro de Entrada e Saída (RFID + MQTT)
Este projeto foi desenvolvido como parte de uma solução para o **Hospital Infantil Sabará**, com o objetivo de monitorar a movimentação de pacientes nas alas hospitalares através de um sensor RFID. Os dados são enviados via protocolo MQTT para um painel de visualização em tempo real.

---

## Funcionalidades

- Leitura de cartões/pulseiras RFID com ESP32  
- Registro automático de entrada e saída de pacientes  
- Envio de dados (ID, horário e status) via MQTT e AWS IOT
- Visualização em tempo real com apps como MyMQTT, MQTT Explorer ou dashboards MQTT e também em cloud com AWS IOT.

## Tecnologias utilizadas

- **ESP32**  
- **MFRC522** (leitor RFID)  
- **ArduinoJson** (formatação dos dados em JSON)  
- **PubSubClient** (envio MQTT)  
- **Wi-Fi** (Wokwi-GUEST ou rede local)  
- **Broker MQTT público**: [HiveMQ](https://broker.hivemq.com)  
- **Dashboard**: MyMQTT app (Android), HiveMQ Web Client
- **AWS IOT CORE**: conexão segura MQTT com TLS

---
## Como Funciona

1. O paciente aproxima a pulseira/cartão RFID do leitor (nesse caso foi utilizado simulação de entrada de um ID atráves da Serial para simulação no wokwi).

2. O sistema identifica se é uma entrada ou saída:

3. Se o ID não está registrado → Entrada

4. Se o ID já está registrado → Saída

5. Um JSON com os dados é publicado no tópico sabara/entradas.
  
6. O dado é enviado tanto para o HiveMQ quanto, agora, também para a AWS IoT Core via MQTT seguro (TLS/certificados). 

7. O dashboard conectado exibe em tempo real.
   
8. Para indicação visual:

- O LED verde acende para indicar entradas.

- O LED vermelho acende para indicar saídas.

- O servo motor abre simulando a abertura de uma porta nas entradas.

## Diagrama

![diagrama_mapa_da_dor](https://github.com/user-attachments/assets/7aa6476e-b046-48a7-8aaf-76d7cd9b80a9)

O diagrama está dividido em três camadas:

### IoT

- Inclui o ESP32 conectado a sensores (RFID, Servo, LED).

- O ESP32 envia dados via Wi-Fi para a nuvem.

### Back-end

- Um broker MQTT (HiveMQ e AWS IoT Core) recebe os dados JSON.

- Esses dados são encaminhados para aplicações que tratam relatórios e históricos.

### Aplicação

- Interfaces Web ou Apps que consomem e exibem os dados em tempo real.

- Possibilidade de relatórios e análises sobre movimentação dos pacientes.

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

AWS IOT

- Conexão AWS IoT Core

- Certificados TLS configurados no ESP32

- Endpoint personalizado fornecido pela AWS

- Porta: 8883

- Uso de bibliotecas seguras (ex: WiFiClientSecure)

É necessário inserir credencias AWS no arquivo certificates.h conforme abaixo:
```
const char* aws_cert_ca = R"EOF(
-----BEGIN CERTIFICATE-----
// INSERIR CERTIFICATE CA
-----END CERTIFICATE-----
)EOF";

const char* aws_cert_crt = R"KEY(
-----BEGIN CERTIFICATE-----
// INSERIR CERTIFICATE CRT
-----END CERTIFICATE-----
)KEY";

const char* aws_cert_private = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
// INSERIR CERTIFICATE PRIVATE
-----END RSA PRIVATE KEY-----
)KEY";
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

## WOKWI
https://wokwi.com/projects/426945753890067457

## Autores

- Lucas Alves Piereti - RM559533
- Rafaela Heer Robinson - RM560249
- Julia Hadja Kfouri Nunes - RM559410
- Witalon Antonio Rodrigues - RM559023

