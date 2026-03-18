#include <LittleFS.h>
#include <EEPROM.h>
#include "players.h"
#include "ui.h"
#include "input.h"
#include "menu.h"

void setup() {
  // Запускаем файловую систему
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }

  uiInit();
 
  showSplash();       // заставка один раз
  inputInit();
  settingsLoad();     // загружаем настройки
  loadGameState();    // Загружаем игру, если есть сохранение
  menuInit();
}


void loop() {
  inputUpdate();  // читает UART, вызывает handleXXX()
  menuUpdate();   // логика меню, таймеры, состояния

}
