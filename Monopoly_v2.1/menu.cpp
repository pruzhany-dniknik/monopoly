// menu.cpp
#include "menu.h"
#include "ui.h"
#include "players.h"
#include <RTClib.h>


#define RTC_FLAG_ADDR 64    // адрес флага "RTC уже инициализирован"
#define RTC_CRC_ADDR 0x08   // Адрес в NV RAM DS1307
#define FW_VERSION_NUM 213  // если надо время синхронизировать - меняем значение
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
int winnerIndex = -1;
long winnerBalance = 0;
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
  // const uint32_t* ptr = (uint32_t*)0x40200000;  // начало прошивки в Flash
  // uint32_t crc = 0xFFFFFFFF;

  // // Определяем реальный размер прошивки через ESP.getSketchSize()
  // uint32_t sketchSize = ESP.getSketchSize();
  // uint32_t words = sketchSize / 4;

  // for (uint32_t i = 0; i < words; i++) {
  //   crc ^= *ptr++;
  //   crc = (crc >> 1) | (crc << 31);
  // }

  // return crc;
   return FW_VERSION_NUM;
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

  // --- Синхронизация RTC только при перепрошивке ---
  if (rtc_ok) {
    uint32_t currentCRC = calcFirmwareCRC();
    uint32_t storedCRC = 0;

    // Читаем сохранённую CRC из NV RAM RTC (адрес 0x08)
    Wire.beginTransmission(0x68);
    Wire.write(0x08);
    if (Wire.endTransmission() == 0) {
      Wire.requestFrom(0x68, 4);
      for (int i = 0; i < 4; i++) {
        if (Wire.available()) {
          storedCRC |= ((uint32_t)Wire.read() << (i * 8));
        }
      }
    }

    // Если CRC не совпадает — прошивка изменилась
    if (storedCRC != currentCRC) {
      Serial.println("Firmware changed → updating RTC time");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

      // Сохраняем новую CRC в NV RAM RTC
      Wire.beginTransmission(0x68);
      Wire.write(0x08);
      for (int i = 0; i < 4; i++) {
        Wire.write((currentCRC >> (i * 8)) & 0xFF);
      }
      Wire.endTransmission();
      Serial.println("RTC updated and CRC saved");
    } else {
      Serial.println("Firmware unchanged → RTC time preserved");
    }
  } else {
    Serial.println("RTC not available, time sync disabled");
  }

  // --- Переход в главное меню ---
  menuState = STATE_MAIN_MENU;
  uiShowMainMenu(gameActive);
  delay(500);
}

void menuUpdate() {
  switch (menuState) {

    case STATE_GAME_WAITCARD:
      {
        // static uint32_t lastAnim = 0;
        // static bool visible = true;
        static uint32_t lastGameTime = 0;
        static uint32_t lastBattery = 0;
        static uint32_t lastClock = 0;
        uint32_t nowMs = millis();

        // --- Мигание строки "Ожидание карты" (раз в 510 мс) ---
        // if (nowMs - lastAnim > 510) {
        //   lastAnim = nowMs;
        //   visible = !visible;
        //   u8g2.setDrawColor(0);
        //   u8g2.drawBox(4, 26, 120, 12);  // очистить строку
        //   u8g2.setDrawColor(1);
        //   if (visible) {
        //     u8g2.drawUTF8(4, 36, "Ожидание карты");
        //   }
        //   u8g2.sendBuffer();
        // }

        // --- Обновление времени игры (раз в 1 сек) ---
        if (nowMs - lastGameTime > 1000) {
          lastGameTime = nowMs;
          uint32_t sec = getGameSeconds();
          uint32_t h = sec / 3600;
          uint32_t m = (sec / 60) % 60;
          uint32_t s = sec % 60;
          char buf[32];
          snprintf(buf, sizeof(buf), "Время игры: %02lu:%02lu:%02lu", h, m, s);
          u8g2.setDrawColor(0);
          u8g2.drawBox(4, 12, 120, 12);  // очистить строку времени
          u8g2.setDrawColor(1);
          u8g2.drawUTF8(4, 24, buf);
          u8g2.sendBuffer();
        }

        // --- Обновление батареи  ---
        if (nowMs - lastBattery > 10030) {
          lastBattery = nowMs;
          float v = readBatteryVoltage();
          int p = batteryPercent(v);
          cachedBatteryPercent = p;
          char buf[16];
          snprintf(buf, sizeof(buf), "[%d%%]", p);
          u8g2.setDrawColor(0);
          u8g2.drawBox(70, 51, 40, 12);  // очистить область батареи
          u8g2.setDrawColor(1);
          u8g2.drawUTF8(82, 60, buf);
          u8g2.sendBuffer();
        }

        // --- Обновление текущего времени (раз в 60 сек) ---
        if (nowMs - lastClock > 60000) {
          lastClock = nowMs;
          DateTime now = safeNow();
          char buf[16];
          snprintf(buf, sizeof(buf), "%02d:%02d", now.hour(), now.minute());
          u8g2.setDrawColor(1);
          u8g2.drawBox(80, 0, 45, 12);  // очистить область часов
          u8g2.setDrawColor(0);
          u8g2.drawUTF8(94, 10, buf);
          u8g2.setDrawColor(1);
          u8g2.sendBuffer();
        }
        break;
      }
    case STATE_SETTINGS_BATTERY_TEST:
      {
        static uint32_t lastUpdate = 0;
        uint32_t nowMs = millis();

        // Обновляем каждые 500 мс
        if (nowMs - lastUpdate > 500) {
          lastUpdate = nowMs;
          uiShowBatteryTest();  // перерисовываем экран с новыми данными
        }
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

  // players[currentRegisteringPlayer].name = strdup(nameBuf);
  static char playerNames[8][16];
  snprintf(playerNames[currentRegisteringPlayer], 16, "Игрок %d", currentRegisteringPlayer + 1);
  players[currentRegisteringPlayer].name = playerNames[currentRegisteringPlayer];
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
    case STATE_GAME_FINISH:
      {
        char line2[32];
        char line3[32];
        snprintf(line2, sizeof(line2), "Игрок %d", winnerIndex + 1);
        snprintf(line3, sizeof(line3), "Баланс: %ld%s", winnerBalance, names[settings.currency]);
        uiShowGame_Result("ПОБЕДИТЕЛЬ:", line2, line3);
        break;
      }
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

    case STATE_SETTINGS_LANGUAGE:
      uiShowSettingsLanguage();
      break;

    case STATE_SETTINGS_RTC_TEST:
      uiShowRTCTest();
      break;

    case STATE_SETTINGS_BATTERY_TEST:
      uiShowBatteryTest();
      break;

    case STATE_SETTINGS_SAVE:
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

      // float v = readBatteryVoltage();// При входе обновляем значение батареи (пока отключим)
      // cachedBatteryPercent = batteryPercent(v);
      uiShowGame_WaitCard(now, getGameSeconds());
      break;

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
