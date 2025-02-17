#include <EmonLib.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
EnergyMonitor emon1;

#define ruido 0.05        // Valor de ruído
#define tensao 220        // Tensão da rede
#define sensorPin A0      // Sensor de corrente
#define calibracao 9.78   // Valor de calibração

char ssid[] = "Sua rede";
char pass[] = "Senha da rede";
const char* mqtt_server = "Server MQTT IP";
const int mqtt_port = 1883;

#define rele 5

void callback(char* topic, byte* payload, int length) {
  
}

void reconnect() {
  // Loop até conectar ao broker MQTT
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado!");
      // Inscreva-se em um tópico, se necessário
      // client.subscribe("seu_topico");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando à rede Wi-Fi: ");
    Serial.println(ssid);
    delay(1000);
  }

  Serial.println("Conectado ao Wi-Fi!");

  pinMode(rele, OUTPUT);
  digitalWrite(rele, HIGH);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  emon1.current(sensorPin, calibracao);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  double Irms = emon1.calcIrms(1480);

  if (Irms <= ruido) {
    Irms = 0;
  }

  double consumo = Irms * tensao;

  Serial.print("Irms:");
  Serial.print(Irms);
  Serial.print(" A   Consumo:");
  Serial.print(consumo);
  Serial.println(" W");

  client.publish("consumption/ESP8266Client", String(consumo).c_str());
  delay(1000);
}
