// ============================================================
//  Perifericos: sensores ultrassonicos, teclado e botao
// ============================================================

#include <NewPing.h>
#include <Keypad.h>
#include "config.h"

// ---- Sensores ultrassonicos (NewPing) -----------------------
NewPing sonarLargada(LARGADA_TRIG, LARGADA_ECHO, MAX_DIST);
NewPing sonarReta(RETA_TRIG, RETA_ECHO, MAX_DIST);

static unsigned long ultimaDetecLargada = 0;
static unsigned long ultimaDetecReta    = 0;

// ---- Teclado matricial 4x4 ----------------------------------
char teclas[KEYPAD_ROWS][KEYPAD_COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte pinosLinhas[KEYPAD_ROWS] = { 13, 12, 14, 27 };
byte pinosColunas[KEYPAD_COLS] = { 26, 25, 33, 32 };
Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas,
                        KEYPAD_ROWS, KEYPAD_COLS);

// ---- Botao bandeira amarela ---------------------------------
static unsigned long ultimoBotao = 0;

void sensorsInit() {
  pinMode(BTN_BANDEIRA, INPUT_PULLUP);
}

// Detecta passagem quando a distancia cai abaixo do limiar,
// respeitando o debounce por sensor
static bool passou(NewPing& sonar, unsigned long& ultimaDetec) {
  unsigned long agora = millis();
  if (agora - ultimaDetec < SENSOR_DEBOUNCE) return false;

  unsigned int dist = sonar.ping_cm();
  if (dist > 0 && dist <= DETEC_DIST) {
    ultimaDetec = agora;
    return true;
  }
  return false;
}

void checkSensorLargada() {
  if (passou(sonarLargada, ultimaDetecLargada)) {
    registrarVolta();
  }
}

void checkSensorReta() {
  if (passou(sonarReta, ultimaDetecReta)) {
    registrarReta();
  }
}

// Botao alterna a solicitacao local de bandeira amarela (g1_ba)
void checkBotaoBandeira() {
  if (digitalRead(BTN_BANDEIRA) == LOW) {
    unsigned long agora = millis();
    if (agora - ultimoBotao > BTN_DEBOUNCE) {
      ultimoBotao = agora;
      solicitacaoBandeira = !solicitacaoBandeira;
      mqttPublish(TOPIC_BA, solicitacaoBandeira ? "1" : "0");
    }
  }
}

// Leitura do teclado durante a inscricao do piloto
//   '#' confirma o nome e publica g1_piloto
//   '*' apaga o ultimo caractere
void atualizarInscricao() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  if (tecla == '#') {
    if (nomePiloto.length() > 0) {
      mqttPublish(TOPIC_PILOTO, nomePiloto);
      telaAguardandoLargada(nomePiloto);
    }
  } else if (tecla == '*') {
    if (nomePiloto.length() > 0) {
      nomePiloto.remove(nomePiloto.length() - 1);
      telaInscricao(nomePiloto);
    }
  } else {
    if (nomePiloto.length() < LCD_COLS - 1) {
      nomePiloto += tecla;
      telaInscricao(nomePiloto);
    }
  }
}
