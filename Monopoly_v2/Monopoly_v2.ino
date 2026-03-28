// LOLIN(WeMos) D1 R1
#include <LittleFS.h>
#include <EEPROM.h>
#include "players.h"
#include "ui.h"
#include "input.h"
#include "menu.h"

void setup() {
  inputInit(); 
  Serial.println(".");
  Serial.println("Start game...");
  // Запускаем файловую систему
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }
  Serial.println("LittleFS init...");

  delay(20);
  EEPROM.begin(128);
  Serial.println("EEPROM init...");
  delay(100);

  settingsLoad();  // загружаем настройки
  uiInit();
  showSplash();  // заставка один раз
  loadGameState();  // Загружаем игру, если есть сохранение
  Serial.println("Load setting...");
  delay(200);
  menuInit();
  Serial.println("menu init...");
  Serial.swap();    // перенос UART на D7 (RX) и D10 (TX)
  delay(100);
}


void loop() {
  inputUpdate();  // читает UART, вызывает handleXXX()
  menuUpdate();   // логика меню, таймеры, состояния
  
}
