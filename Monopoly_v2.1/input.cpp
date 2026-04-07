#include "input.h"
#include "menu.h"
#include "players.h"
#include "ui.h"

// String inputLine;

#define BUF_SIZE 64
char buf[BUF_SIZE];
uint8_t pos = 0;


void inputUpdate() {
  if (Serial.available() > 500) {
    while (Serial.available()) Serial.read();
    return;
  }
  while (Serial.available()) {
    char c = Serial.read();

    // конец строки
    if (c == '\n' || c == '\r') {
      if (pos > 0) {
        buf[pos] = 0;  // завершаем строку
        processLine(buf);
        pos = 0;
      }
      continue;
    }

    // защита от переполнения
    if (pos < BUF_SIZE - 1) {
      buf[pos++] = c;
    } else {
      pos = 0;  // сброс при мусоре
    }
  }
}
void processLine(const char* line) {
 
  if (strncmp(line, "KEY ", 4) == 0) {
    menuHandleKey(line + 4);
    return;
  }

  if (strncmp(line, "CARD ", 5) == 0) {
    byte uid[4];
    if (parseUID_C(line + 5, uid)) {
      handleCard(uid);
    }
    return;
  }
}
bool parseUID_C(const char* s, byte out[4]) {
  for (int i = 0; i < 4; i++) {
    char part[3] = { s[0], s[1], 0 };
    out[i] = strtol(part, NULL, 16);

    // пропускаем 2 hex + пробел
    s += 2;
    if (*s == ' ') s++;
  }
  return true;
}
void handleCard(byte uid[4]) {

  if (menuState == STATE_NEWGAME_REGCARD) {
    handleCardInNewGameWizard(uid);
    return;
  }

  if (gameActive) {
    int idx = findPlayerByUID(uid);
    if (idx >= 0) {
      bool bankrupt = players[idx].eliminated || (settings.autoEndGame && players[idx].balance == 0);

      if (bankrupt) {
        showBankruptFlash(idx);
        return;
      }

      activePlayerIndex = idx;
      setMenuState(STATE_GAME_PLAYERMENU);
    }
  }
}


float readBatteryVoltage() {
  int raw = analogRead(A0);
  float voltageAtA0 = (raw / 1023.0) * 3.3;  // напряжение на пине A0
   // Коэффициент делителя (R1+R2)/R2
  // Для R1=200k, R2=22k: (200+22)/22 = 10.09
  // float dividerRatio = (180.0 + 22.0) / 22.0;  // ≈ 10.09
  float dividerRatio =10.5;
  float batteryVoltage = voltageAtA0 * dividerRatio;
  return batteryVoltage;
}

int batteryPercent(float v) {
  if (v >= 8.30) return 100;
  if (v >= 8.10) return 96;
  if (v >= 7.90) return 88;
  if (v >= 7.70) return 78;
  if (v >= 7.50) return 65;
  if (v >= 7.30) return 50;
  if (v >= 7.10) return 35;
  if (v >= 6.90) return 20;
  if (v >= 6.70) return 10;
  if (v >= 6.50) return 5;
  if (v >= 6.40) return 4;
  if (v >= 6.30) return 2;
  if (v >= 6.20) return 1;
  return 0;
  // if (v <= 6.20) return 0;
  // return (int)((v - 6.2) * (100.0 / 2.1));
}
