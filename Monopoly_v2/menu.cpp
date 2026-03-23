// menu.cpp
#include "menu.h"
#include "ui.h"
#include "players.h"
#include <RTClib.h>


#define RTC_FLAG_ADDR 64  // адрес флага "RTC уже инициализирован"

// ---------------------------------------------------------
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ---------------------------------------------------------
MenuState menuState = STATE_MAIN_MENU;
MenuState returnFromBalances = STATE_GAME_WAITCARD;
MenuState amountNextState = STATE_GAME_TRANSFER_SUM;

int activePlayerIndex = -1;
int currentRegisteringPlayer = 0;
int selectedStartBalance = 1500;
int transferTargetIndex = -1;
long transferAmount = 0;
long presetAmounts[5] = { 100, 250, 500, 1000, 2000 };

bool helpMode = false;
int helpPageIndex = 0;
HelpPage helpPages[] = {
  { "ОБЩЕЕ:", "Это электронный терминал", "для настольной игры", "Монополия.", "Управление картами" },
  { "УПРАВЛЕНИЕ:", "Стрелки - листание", "Ent - подтверждение", "Esc - назад", "" },
  { "RFID:", "Каждый игрок имеет", "свою карту.", "Приложите карту", "к считывателю" },
  { "ПОДСВЕТКА:", "F1 - яркость", "Автоотключение", "через 2 минуты", "" },
  { "ИГРОВОЙ РЕЖИМ:", "Выберите игрока", "Введите сумму", "Подтвердите", "" },
  { "БАЛАНС:", "F2 - просмотр", "балансов игроков", "", "" },
  { "О ПРОЕКТЕ:", "", "", "", "" }
};
int helpPageCount = sizeof(helpPages) / sizeof(helpPages[0]);


RTC_DS1307 rtc;
bool rtc_ok = false;  // флаг: RTC работает или нет
bool gameActive = false;
DateTime gameStartTime;  // старая переменная, чтобы проект собирался

// Игровой таймер — теперь полностью независим от RTC
unsigned long gameStartMillis = 0;
unsigned long getGameSeconds() {
  return (millis() - gameStartMillis) / 1000;
}

// Безопасное чтение времени
DateTime safeNow() {
  if (rtc_ok) return rtc.now();
  return DateTime(2026, 1, 1, 0, 0, 0);  // нули, если RTC нет
}

// ---------------------------------------------------------
// Чтение секунд напрямую из DS1307 (для проверки работы)
// ---------------------------------------------------------
uint8_t readSecondsRaw() {
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return 0xFF;

  Wire.requestFrom(0x68, 1);
  if (!Wire.available()) return 0xFF;

  return Wire.read();
}

// ---------------------------------------------------------
// ВОССТАНОВЛЕНИЕ I2C-ШИНЫ (если SDA зажата)
// ---------------------------------------------------------
void i2cBusRecovery() {
  pinMode(5, OUTPUT);  // SCL = GPIO5
  pinMode(4, OUTPUT);  // SDA = GPIO4

  digitalWrite(4, HIGH);
  for (int i = 0; i < 16; i++) {
    digitalWrite(5, HIGH);
    delayMicroseconds(5);
    digitalWrite(5, LOW);
    delayMicroseconds(5);
  }

  // STOP condition
  digitalWrite(4, LOW);
  delayMicroseconds(5);
  digitalWrite(5, HIGH);
  delayMicroseconds(5);
  digitalWrite(4, HIGH);
  delayMicroseconds(5);
}

// ---------------------------------------------------------
// ИНИЦИАЛИЗАЦИЯ МЕНЮ И RTC
// ---------------------------------------------------------
bool initRTC() {
  Serial.println("RTC: init...");
  Wire.beginTransmission(0x68);  // 1. Проверяем I2C
  if (Wire.endTransmission() != 0) {
    Serial.println("RTC: NO I2C RESPONSE");
    return false;
  }
  // 2. Проверяем rtc.begin()
  if (!rtc.begin()) {
    Serial.println("RTC: rtc.begin() FAILED");
    return false;
  }
  Serial.println("RTC: OK");
  return true;
}

// ---------------------------------------------------------
// CRC прошивки — определяет, была ли перепрошивка
// ---------------------------------------------------------
uint32_t calcFirmwareCRC() {
  const uint32_t* ptr = (uint32_t*)0x40200000;  // начало прошивки в Flash
  uint32_t crc = 0xFFFFFFFF;

  // считаем 512 KB (максимальный размер прошивки ESP8266)
  for (size_t i = 0; i < 0x80000; i += 4) {
    crc ^= *ptr++;
    crc = (crc >> 1) | (crc << 31);
  }
  return crc;
}

void menuInit() {
  delay(200);  // даём DS1307 проснуться

  // --- I2C ---
  Wire.begin(4, 5);  // SDA=GPIO4 (D4), SCL=GPIO5 (D3)
  delay(50);

  // --- Проверяем ACK RTC ---
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() != 0) {
    Serial.println("RTC: no ACK, trying recovery...");
    i2cBusRecovery();
    delay(50);
    Wire.begin(4, 5);
    delay(50);
  }

  // --- Проверяем rtc.begin() ---
  rtc_ok = initRTC();

  // --- Автоматическая синхронизация RTC при перепрошивке ---
  uint32_t storedCRC = 0;
  EEPROM.get(RTC_FLAG_ADDR, storedCRC);

  uint32_t currentCRC = calcFirmwareCRC();

  if (storedCRC != currentCRC) {
    Serial.println("Firmware changed → updating RTC time");

    // Устанавливаем время прошивки
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Сохраняем CRC прошивки
    EEPROM.put(RTC_FLAG_ADDR, currentCRC);
    EEPROM.commit();
  } else {
    Serial.println("Firmware unchanged → RTC not touched");
  }

  // --- Переход в главное меню ---
  menuState = STATE_MAIN_MENU;
  uiShowMainMenu(gameActive);
  delay(500);
}

// ---------------------------------------------------------
// ОБНОВЛЕНИЕ ЭКРАНОВ И ТАЙМЕРОВ
// ---------------------------------------------------------
void menuUpdate() {
  DateTime now = safeNow();  // безопасное чтение времени
  switch (menuState) {
   
case STATE_GAME_WAITCARD:
      {
        uint32_t gameSec = getGameSeconds();
        uiShowGame_WaitCard(now, gameSec);
        break;
      }
    
    case STATE_GAME_TRANSFER_SELECTPLAYER:
      uiShowGame_SelectPlayer();
      break;

    case STATE_GAME_SUM_PRESET:
      uiShowGame_SumPreset();
      break;

    case STATE_GAME_SUM_MANUAL:
      uiShowGame_SumManual(transferAmount);
      break;

    case STATE_GAME_TRANSFER_SUM:
    case STATE_GAME_BANK_PAY:
    case STATE_GAME_BANK_GET:
      uiShowGame_EnterAmount(transferAmount);
      break;

    case STATE_GAME_BALANCES:
      {
        int bal[8];
        for (int i = 0; i < selectedPlayerCount; i++)
          bal[i] = players[i].balance;
        uiShowGame_Balances(selectedPlayerCount, bal);
        break;
      }

    default:
      break;
  }
}
void handleCardInNewGameWizard(byte uid[4]) {
  for (int i = 0; i < currentRegisteringPlayer; i++) {
    if (memcmp(players[i].uid, uid, 4) == 0) {
      uiShowNewGame_RegCardError();
      return;
    }
  }

  memcpy(players[currentRegisteringPlayer].uid, uid, 4);

  static char nameBuf[16];
  snprintf(nameBuf, sizeof(nameBuf), "Игрок %d", currentRegisteringPlayer + 1);
  players[currentRegisteringPlayer].name = strdup(nameBuf);

  uiShowNewGame_RegCardOK(currentRegisteringPlayer, uid);

  currentRegisteringPlayer++;
}

// ---------------------------------------------------------
// ПЕРЕКЛЮЧЕНИЕ СОСТОЯНИЙ
// ---------------------------------------------------------
void setMenuState(MenuState newState) {
  menuState = newState;

  DateTime now = safeNow();  // безопасное чтение времени

  switch (newState) {

    case STATE_SETTINGS_MENU:
      uiShowSettingsMenu();
      break;
    case STATE_SETTINGS_GAME:
      uiShowSettingsGame();
      break;

    case STATE_SETTINGS_SYSTEM:
      uiShowSettingsSystem();
      break;

    case STATE_SETTINGS_SYSINFO:
      uiShowSettingsSysInfo();
      break;

    case STATE_SETTINGS_SERVICE:
      uiShowServiceMenu();
      break;

    case STATE_SETTINGS_CURRENCY:
      uiShowSettingsCurrency();
      break;

    case STATE_SETTINGS_RTC_TEST:
      uiShowRTCTest();
      break;

    case STATE_SETTINGS_BATTERY_TEST:
      uiShowBatteryTest();
      break;

    case STATE_SETTINGS_SAVE:
      Serial.println("Save begin...");
      settingsSave();
      drawScreen("Настройки", "Сохранено!", "", "", "");
      delay(1000);
      uiShowSettingsMenu();
      menuState = STATE_SETTINGS_MENU;
      break;

    case MENU_HELP:
      helpMode = true;
      helpPageIndex = 0;
      drawHelpScreen();
      break;

    case STATE_MAIN_MENU:
      uiShowMainMenu(gameActive);
      break;

    // --- Мастер новой игры ---
    case STATE_NEWGAME_NUMPLAYERS:
      currentRegisteringPlayer = 0;
      uiShowNewGame_NumPlayers(selectedPlayerCount);
      break;

    case STATE_NEWGAME_REGCARD:
      uiShowNewGame_RegCard(currentRegisteringPlayer);
      break;

    case STATE_NEWGAME_STARTBALANCE:
      uiShowNewGame_StartBalance(selectedStartBalance);
      break;

    case STATE_NEWGAME_CONFIRM:
      uiShowNewGame_Confirm(selectedPlayerCount, selectedStartBalance);
      gameStartMillis = millis();
      gameActive = true;
      break;

    // --- Игровой режим ---
    case STATE_GAME_WAITCARD:
      {
        uint32_t gameSec = getGameSeconds();
        uiShowGame_WaitCard(now, gameSec);
        break;
      }

    case STATE_GAME_PLAYERMENU:
      uiShowGame_PlayerMenu(players[activePlayerIndex].name,
                            players[activePlayerIndex].balance);
      break;

    case STATE_GAME_TRANSFER_SELECTPLAYER:
      uiShowGame_SelectPlayer();
      break;

    case STATE_GAME_SUM_PRESET:
      uiShowGame_SumPreset();
      break;

    case STATE_GAME_SUM_MANUAL:
      transferAmount = 0;
      uiShowGame_SumManual(transferAmount);
      break;

    case STATE_GAME_TRANSFER_SUM:
    case STATE_GAME_BANK_PAY:
    case STATE_GAME_BANK_GET:
      uiShowGame_EnterAmount(transferAmount);
      break;

    case STATE_GAME_BALANCES:
      {
        int bal[8];
        for (int i = 0; i < selectedPlayerCount; i++)
          bal[i] = players[i].balance;
        uiShowGame_Balances(selectedPlayerCount, bal);
        break;
      }

    default:
      break;
  }
}
