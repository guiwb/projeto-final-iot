// ============================================================
//  Pista de Corrida - Grupo 1 (Projeto IoT II / ESP32)
//
//  Cliente MQTT que registra o piloto, cronometra voltas,
//  calcula a velocidade na reta e trata bandeira amarela.
//
//  Tabs do sketch (Arduino IDE):
//    - pista_corrida.ino : estado global, setup, loop, FSM
//    - mqtt.ino          : Wi-Fi + MQTT (mock ou real)
//    - display.ino       : telas do LCD 16x2 I2C
//    - sensors.ino       : sensores, teclado e botao
//    - config.h          : pinos, topicos e constantes
//
//  Tempos em milissegundos; velocidade em m/s (reta de 8 m).
//
//  Modo mock (USE_MQTT_MOCK=1): publica no Serial e injeta
//  mensagens do broker digitando no Serial Monitor, ex.:
//      br_nvoltas 5
//      br_largada
//      br_iba
//      br_fba
//      br_fim
// ============================================================

#include <NewPing.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// ---- Estados da maquina --------------------------------------
enum Estado {
  AGUARDANDO_CONFIGURACAO,
  INSCRICAO,
  CORRIDA
};

// ---- Estado global compartilhado entre as tabs ---------------
Estado estado = AGUARDANDO_CONFIGURACAO;

String        nomePiloto          = "";
int           totalVoltas         = 0;
int           voltaAtual          = 0;
unsigned long horarioLargada      = 0;
unsigned long ultimaPassagemLargada = 0;

bool bandeiraAtiva     = false;  // bandeira amarela global (br_iba/br_fba)
bool voltaComBandeira  = false;  // volta atual "contaminada"
bool solicitacaoBandeira = false; // pedido local via botao (g1_ba)
bool corridaEmAndamento = false;

// ============================================================
//  Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  displayInit();
  sensorsInit();
  mqttSetup();

  irParaAguardando();
}

// ============================================================
//  Loop
// ============================================================
void loop() {
  mqttLoop();        // mantem conexao + processa mensagens recebidas
  checkBotaoBandeira();

  switch (estado) {
    case AGUARDANDO_CONFIGURACAO:
      // aguarda br_nvoltas (tratado no callback)
      break;

    case INSCRICAO:
      atualizarInscricao();   // le teclado e monta o nome
      break;

    case CORRIDA:
      // bandeira amarela ativa: nao contabiliza voltas
      if (corridaEmAndamento && !bandeiraAtiva) {
        checkSensorLargada();
      }
      break;
  }
}

// ============================================================
//  Transicoes de estado
// ============================================================
void irParaAguardando() {
  estado = AGUARDANDO_CONFIGURACAO;
  nomePiloto = "";
  totalVoltas = 0;
  voltaAtual = 0;
  horarioLargada = 0;
  ultimaPassagemLargada = 0;
  bandeiraAtiva = false;
  voltaComBandeira = false;
  solicitacaoBandeira = false;
  corridaEmAndamento = false;
  digitalWrite(LED_BANDEIRA, LOW);
  telaAguardando();
}

void irParaInscricao(int voltas) {
  totalVoltas = voltas;
  nomePiloto = "";
  estado = INSCRICAO;
  telaInscricao(nomePiloto);
}

void irParaCorrida() {
  horarioLargada = millis();
  ultimaPassagemLargada = horarioLargada;
  voltaAtual = 0;
  voltaComBandeira = false;
  corridaEmAndamento = true;
  estado = CORRIDA;
  telaCorrida(nomePiloto, voltaAtual, totalVoltas);
}

// ============================================================
//  Logica da corrida
// ============================================================

// Passagem pela linha de largada/chegada -> fecha uma volta
void registrarVolta() {
  unsigned long agora = millis();
  unsigned long tempoVolta;

  if (voltaAtual == 0) {
    tempoVolta = agora - horarioLargada;        // primeira volta
    voltaAtual = 1;
  } else {
    tempoVolta = agora - ultimaPassagemLargada; // demais voltas
    voltaAtual++;
  }
  ultimaPassagemLargada = agora;

  // volta contaminada por bandeira amarela publica 0
  if (voltaComBandeira) {
    mqttPublish(TOPIC_TVOLTA, "0");
    voltaComBandeira = false;
  } else {
    mqttPublish(TOPIC_TVOLTA, String(tempoVolta));
  }

  telaCorrida(nomePiloto, voltaAtual, totalVoltas);

  if (voltaAtual >= totalVoltas) {
    finalizarCorrida();
  }
}

// Atingiu o total de voltas: encerra a contabilizacao e exibe no LCD
void finalizarCorrida() {
  corridaEmAndamento = false;
  digitalWrite(LED_BANDEIRA, LOW);
  telaCorridaConcluida(nomePiloto, totalVoltas);
}

// ============================================================
//  Tratamento das mensagens do broker
// ============================================================
void handleMessage(const String& topic, const String& payload) {
  if (topic == TOPIC_NVOLTAS) {
    if (estado == AGUARDANDO_CONFIGURACAO) {
      irParaInscricao(payload.toInt());
    }
  } else if (topic == TOPIC_LARGADA) {
    if (estado == INSCRICAO) {
      irParaCorrida();
    }
  } else if (topic == TOPIC_FIM) {
    if (estado == CORRIDA) {
      corridaEmAndamento = false;
      telaFim();
      delay(2000);
      irParaAguardando();
    }
  } else if (topic == TOPIC_IBA) {
    bandeiraAtiva = true;
    digitalWrite(LED_BANDEIRA, HIGH);
    if (estado == CORRIDA) {
      voltaComBandeira = true;
      telaBandeira(voltaAtual, totalVoltas);
    }
  } else if (topic == TOPIC_FBA) {
    bandeiraAtiva = false;
    digitalWrite(LED_BANDEIRA, LOW);
    if (estado == CORRIDA) {
      telaCorrida(nomePiloto, voltaAtual, totalVoltas);
    }
  }
}
