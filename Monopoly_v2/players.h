#pragma once
#pragma pack(push, 1)
#include <Arduino.h>
#include <RTClib.h>
#include "ui.h"
#include <EEPROM.h>
#include <LittleFS.h>

#define SETTINGS_MAGIC 0xCAFEBABE
#define SETTINGS_VERSION 1
#define FW_VERSION "2.13"



extern int startBalance;                  // Стартовый капитал
extern const unsigned long eventTimeout;  // Таймаут для событий (мс)
extern int selectedPlayerCount;           // Количество игроков в текущей игре
extern uint32_t lastBatteryCheck;
extern int cachedBatteryPercent;


// ---------------------------------------------------------
// СТРУКТУРА ИГРОКА
// ---------------------------------------------------------
struct Player {
  const char* name;
  byte uid[4];
  long balance;
  bool eliminated;
};

extern Player players[8];  // Глобальный массив игроков

struct Settings {
  uint32_t magic;    // маркер валидности
  uint16_t version;  // версия структуры

  long maxBalance;         // 1. Максимальный баланс
  bool confirmLargeOps;    // 2. Подтверждение крупных операций
  long largeOpThreshold;   // 3. Порог крупной операции
  bool autoEndGame;        // 4. Автозавершение игры

  byte currency;           // 0=-, 1=$, 2=Е, 3=Р
  bool wifiEnabled;        // вкл/выкл вайфай
};

extern Settings settings;
extern bool gameActive;         // флаг активности игры
extern DateTime gameStartTime;  // время начала игры
extern const char* names[];     // валюта
extern const char* currencyNames[];

// ---------------------------------------------------------
// ФУНКЦИИ РАБОТЫ С ИГРОКАМИ
// ---------------------------------------------------------

// Поиск игрока по UID карты
int findPlayerByUID(byte uid[4]);

// Сброс балансов при создании новой игры
void resetAllBalances();


// Операции
long doTransfer(int from, int to, long amount);
long doTransferAll(int from, long amount);
long doPayBank(int from, long amount);
long doGetBank(int from, long amount);
bool canAdd(long current, long delta, long maxLimit);
void playerEliminated(int idx);

bool checkAutoEndGame();
extern const long maxOperationAmount;

// Сохранение/загрузка игры (LittleFS)
void saveGameState();
void loadGameState();
// Сохранение/загрузка настроек
void settingsLoad();
void settingsSave();
