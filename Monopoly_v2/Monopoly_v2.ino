// LOLIN(WeMos) D1 R1
// Monopoly_v2.ino

#define SERIAL_RX_BUFFER_SIZE 512
#include <LittleFS.h>
#include <EEPROM.h>
#include "players.h"
#include "ui.h"
#include "input.h"
#include "menu.h"

void setup() {
  Serial.begin(19200);
  Serial.println(".");
  Serial.println("Start game...");
  delay(100);
  // Запускаем файловую систему
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }
  Serial.println("LittleFS init...");
  delay(100);
  EEPROM.begin(128);
  Serial.println("EEPROM init...");
  delay(100);
  settingsLoad();  // загружаем настройки
  uiInit();
  unsigned long start = micros();
  showSplash();  // заставка
  unsigned long duration = micros() - start;
  Serial.print("drawScreen time: ");
  Serial.print(duration / 1000);
  Serial.println(" ms ");
  delay(2000);
  loadGameState();  // Загружаем игру, если есть сохранение
  Serial.println("Load setting...");
  delay(200);
  menuInit();
  Serial.println("menu init...");
  Serial.swap();  // перенос UART на D7 (RX) и D10 (TX)
  delay(100);
}


void loop() {
  inputUpdate();  // читает UART, вызывает handleXXX()
  menuUpdate();   // логика меню, таймеры, состояния
}
