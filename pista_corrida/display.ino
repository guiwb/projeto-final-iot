// ============================================================
//  Telas do LCD 16x2 I2C (LiquidCrystal_I2C)
// ============================================================

#include <LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void displayInit() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// Imprime duas linhas, limpando o conteudo anterior
void displayLinhas(const String& linha1, const String& linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1.substring(0, LCD_COLS));
  lcd.setCursor(0, 1);
  lcd.print(linha2.substring(0, LCD_COLS));
}

void telaAguardando() {
  displayLinhas("Aguardando", "config. corrida");
}

void telaInscricao(const String& nome) {
  displayLinhas("Piloto:", nome + "_");
}

void telaAguardandoLargada(const String& nome) {
  displayLinhas("Piloto: " + nome, "Aguard. largada");
}

void telaCorrida(const String& nome, int volta, int total) {
  displayLinhas("Piloto: " + nome, "Volta: " + String(volta) + "/" + String(total));
}

void telaBandeira(int volta, int total) {
  displayLinhas("BANDEIRA AMARELA", "Volta: " + String(volta) + "/" + String(total));
}

void telaFim() {
  displayLinhas("CORRIDA", "FINALIZADA");
}
