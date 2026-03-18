#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "ui.h"
#include <EEPROM.h>
#include <LittleFS.h>

// Стартовый капитал
extern int startBalance;

// Таймаут для событий (мс)
extern const unsigned long eventTimeout;

// Количество игроков в текущей игре
extern int selectedPlayerCount;

// ---------------------------------------------------------
// СТРУКТУРА ИГРОКА
// ---------------------------------------------------------
struct Player {
  const char* name;
  byte uid[4];
  long balance;
};

// Глобальный массив игроков (объявлен в players.cpp)
extern Player players[8];

struct Settings {
    long maxBalance;          // 1. Максимальный баланс
    bool confirmLargeOps;     // 2. Подтверждение крупных операций
    long largeOpThreshold;    // 3. Порог крупной операции
    bool autoEndGame;         // 4. Автозавершение игры

    byte language;            // → системные
    int backlightTimeout;     // → системные
    byte defaultBrightness;   // → системные
    bool wifiEnabled;         // → системные
};

extern Settings settings;

// ---------------------------------------------------------
// СТРУКТУРА ПОСЛЕДНЕГО ДЕЙСТВИЯ (для отмены)
// ---------------------------------------------------------
struct LastAction {
  bool valid;
  int type;        // 1 = transfer, 2 = pay bank, 3 = get bank
  int fromPlayer;  // индекс игрока
  int toPlayer;    // индекс игрока или -1 (банк)
  long amount;     // сумма операции
};

// Объявление глобальной переменной (реализована в menu.cpp)
extern LastAction lastAction;

extern bool gameActive;          // флаг активности игры
extern DateTime gameStartTime;   // время начала игры

// ---------------------------------------------------------
// ФУНКЦИИ РАБОТЫ С ИГРОКАМИ
// ---------------------------------------------------------

// Поиск игрока по UID карты
int findPlayerByUID(byte uid[4]);

// Сброс балансов при создании новой игры
void resetAllBalances();

// Запись последнего действия
void saveLastAction(int type, int from, int to, long amount);

// Отмена последнего действия
void undoLastAction();

// Операции
long doTransfer(int from, int to, long amount);
long doPayBank(int from, long amount);
long doGetBank(int from, long amount);
bool canAdd(long current, long delta, long maxLimit);
void playerEliminated(int idx);
void checkAutoEndGame() ;
extern const long maxOperationAmount;

// Сохранение/загрузка игры (LittleFS)
void saveGameState();
void loadGameState();
// Сохранение/загрузка настроек 
void settingsLoad();
void settingsSave();
