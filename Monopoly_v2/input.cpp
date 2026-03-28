#include "input.h"
#include "menu.h"
#include "players.h"
#include "ui.h"


String inputLine;

void inputInit() {
  Serial.begin(19200);
  
  delay(500);
}

#define BUF_SIZE 64
char buf[BUF_SIZE];
uint8_t pos = 0;


void inputUpdate() {

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
  // Serial.print("LINE: ");
  // Serial.println(line);

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
  float v = (raw / 1023.0) * 1.0;  // ESP ADC 1.0V max
  return v * 2.0;                  // делитель 100k/100k
}

int batteryPercent(float v) {
  if (v >= 4.20) return 100;
  if (v <= 3.00) return 0;
  return (int)((v - 3.00) * (100.0 / 1.20));  // линейно 3.0–4.2 В
}
