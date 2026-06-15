// ============================================================
//  Camada MQTT - mock (Serial) ou broker real (PubSubClient)
//  Selecionada por USE_MQTT_MOCK em config.h
// ============================================================

#include "config.h"

#if USE_MQTT_MOCK
// ------------------------------------------------------------
//  MOCK: publica no Serial e injeta mensagens do broker
//  digitando "<topico> <payload>" no Serial Monitor.
// ------------------------------------------------------------

void mqttSetup() {
  Serial.println(F("[MQTT-MOCK] pronto. Digite: <topico> <payload>"));
  Serial.println(F("  ex: br_nvoltas 5 | br_largada | br_iba | br_fba | br_fim"));
}

void mqttPublish(const String& topic, const String& payload) {
  Serial.print(F("[PUB] "));
  Serial.print(topic);
  Serial.print(' ');
  Serial.println(payload);
}

void mqttLoop() {
  if (!Serial.available()) return;

  String linha = Serial.readStringUntil('\n');
  linha.trim();
  if (linha.length() == 0) return;

  int sep = linha.indexOf(' ');
  String topic   = (sep < 0) ? linha : linha.substring(0, sep);
  String payload = (sep < 0) ? ""    : linha.substring(sep + 1);
  payload.trim();

  Serial.print(F("[SUB] "));
  Serial.print(topic);
  Serial.print(' ');
  Serial.println(payload);

  handleMessage(topic, payload);
}

#else
// ------------------------------------------------------------
//  REAL: Wi-Fi + PubSubClient
// ------------------------------------------------------------

#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

static void wifiConnect() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print(F("[WiFi] conectando"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print(F("\n[WiFi] IP: "));
  Serial.println(WiFi.localIP());
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String p;
  p.reserve(length);
  for (unsigned int i = 0; i < length; i++) p += (char)payload[i];
  handleMessage(t, p);
}

static void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print(F("[MQTT] conectando..."));
    bool ok = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
    if (ok) {
      Serial.println(F(" ok"));
      mqttClient.subscribe(TOPIC_NVOLTAS);
      mqttClient.subscribe(TOPIC_LARGADA);
      mqttClient.subscribe(TOPIC_FIM);
      mqttClient.subscribe(TOPIC_IBA);
      mqttClient.subscribe(TOPIC_FBA);
    } else {
      Serial.print(F(" falhou rc="));
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void mqttSetup() {
  wifiConnect();
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(onMqttMessage);
}

void mqttPublish(const String& topic, const String& payload) {
  mqttClient.publish(topic.c_str(), payload.c_str());
}

void mqttLoop() {
  wifiConnect();
  if (!mqttClient.connected()) mqttReconnect();
  mqttClient.loop();
}

#endif // USE_MQTT_MOCK
