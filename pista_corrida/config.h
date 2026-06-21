#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  Configuracao central do projeto - Pista de Corrida (G1)
// ============================================================

// ---- Chave mock / broker real -------------------------------
// 1 = simula MQTT pelo Serial (sem broker / sem rede)
// 0 = usa PubSubClient conectado ao broker real
#define USE_MQTT_MOCK 1

// ---- Wi-Fi (preenchido depois) ------------------------------
#define WIFI_SSID     "SUA_REDE"
#define WIFI_PASSWORD "SUA_SENHA"

// ---- Broker MQTT (preenchido depois) ------------------------
#define MQTT_HOST      "broker.exemplo.com"
#define MQTT_PORT      1883
#define MQTT_USER      ""
#define MQTT_PASS      ""
#define MQTT_CLIENT_ID "g1-pista"

// ---- Topicos publicados -------------------------------------
#define TOPIC_PILOTO "g1_piloto"
#define TOPIC_BA     "g1_ba"
#define TOPIC_TVOLTA "g1_tvolta"   // ms

// ---- Topicos assinados --------------------------------------
#define TOPIC_NVOLTAS "br_nvoltas"
#define TOPIC_LARGADA "br_largada"
#define TOPIC_FIM     "br_fim"
#define TOPIC_IBA     "br_iba"
#define TOPIC_FBA     "br_fba"

// ---- LCD 16x2 I2C -------------------------------------------
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
// I2C padrao do ESP32: SDA = 21, SCL = 22

// ---- Sensores ultrassonicos (NewPing) -----------------------
#define LARGADA_TRIG 2
#define LARGADA_ECHO 4
#define MAX_DIST     200    // cm - alcance maximo do NewPing
#define DETEC_DIST   15     // cm - passagem detectada abaixo deste valor
#define SENSOR_DEBOUNCE 500 // ms - intervalo minimo entre passagens

// ---- Botao bandeira amarela ---------------------------------
#define BTN_BANDEIRA  5
#define BTN_DEBOUNCE  200   // ms
#define LED_BANDEIRA  12

// ---- Teclado matricial 4x4 ----------------------------------
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
// Pinos das linhas e colunas (definidos em sensors.ino)

#endif // CONFIG_H
