#include "players.h"

// Значения по умолчанию
int startBalance = 1500;                  // стартовый капитал
const unsigned long eventTimeout = 2000;  // 2 секунды
int selectedPlayerCount = 2;              // по умолчанию 2 игрока
const long maxOperationAmount = 8000;     // или любое твоё значение
uint32_t lastBatteryCheck = 0;
int cachedBatteryPercent = 100;

const char* names[] = { "", "$", "Е", "р" };
const char* currencyNames[] = { "нет валюты", "доллар", "евро", "рубль" };

// ---------------------------------------------------------
// ГЛАВНЫЙ МАССИВ ИГРОКОВ
// ---------------------------------------------------------
Player players[8];

Settings settings = {
  SETTINGS_MAGIC,
  SETTINGS_VERSION,

  29999,  // maxBalance
  true,   // confirmLargeOps
  5000,   // largeOpThreshold
  true,   // autoEndGame
  0,      // currency
  false  // wifi
};

extern int selectedPlayerCount;
extern int startBalance;
extern bool gameActive;
extern DateTime gameStartTime;

// ---------------------------------------------------------
// ПОИСК ИГРОКА ПО UID
// ---------------------------------------------------------
int findPlayerByUID(byte uid[4]) {
  for (int i = 0; i < selectedPlayerCount; i++) {
    if (memcmp(players[i].uid, uid, 4) == 0) return i;
  }
  return -1;
}

// ---------------------------------------------------------
// СБРОС БАЛАНСОВ
// ---------------------------------------------------------
void resetAllBalances() {
  for (int i = 0; i < selectedPlayerCount; i++) {
    players[i].balance = startBalance;
    players[i].eliminated = false;
  }
}

bool canAdd(long current, long delta, long maxLimit) {
  return (current + delta <= maxLimit);
}

// пометка выбывшего игрока при нулевом балансе
void playerEliminated(int idx) {
  // Можно добавить флаг eliminated, если понадобится
  // Пока просто оставляем баланс = 0
}

// пометка автозавершения игры
bool checkAutoEndGame() {
  if (!settings.autoEndGame) return false;

  int alive = 0;
  int lastAlive = -1;

  for (int i = 0; i < selectedPlayerCount; i++) {
    if (!players[i].eliminated && players[i].balance > 0) {
      alive++;
      lastAlive = i;
    }
  }

  if (alive <= 1) {
    char line2[32];
    snprintf(line2, sizeof(line2), "Игрок %d", lastAlive + 1);
    long winnerBalance = players[lastAlive].balance;
    char line3[32];
    snprintf(line3, sizeof(line3), "Баланс: %ld", winnerBalance);
    uiShowGame_Result("ПОБЕДИТЕЛЬ", line2, line3);

    // сброс игры
    for (int i = 0; i < selectedPlayerCount; i++) {
      players[i].balance = 0;
    }

    if (LittleFS.exists("/game.dat")) { LittleFS.remove("/game.dat"); }
    gameActive = false;
    // lastAction.valid = false;
    setMenuState(STATE_GAME_FINISH);  // ПЕРЕХОД В ФИНАЛ
    return true;
  }

  return false;
}

// ---------------------------------------------------------
// ОПЕРАЦИИ
// ---------------------------------------------------------

long doTransfer(int from, int to, long amount) {
  // if (amount > settings.largeOpThreshold) return 0;
  if (amount > maxOperationAmount) return 0;
  if (from == to) return 0;
  if (amount <= 0) return 0;
  // Ограничение максимальной суммы перевода
  if (amount > settings.maxBalance) return 0;
  long original = amount;
  // Если денег не хватает — переводим всё, что есть
  if (players[from].balance < amount) {
    amount = players[from].balance;
    players[from].balance = 0;
    playerEliminated(from);
  } else {
    players[from].balance -= amount;
  }
  // Ограничение максимального баланса получателя
  long allowed = settings.maxBalance - players[to].balance;
  if (allowed < amount) {
    amount = allowed;
  }
  players[to].balance += amount;
  // saveLastAction(1, from, to, amount);
  if (checkAutoEndGame()) {
    return amount;  // игра завершена, дальше UI сам уже всё сделал
  }
  return amount;  // ← возвращаем реальную сумму
}

long doTransferAll(int from, long amount) {
  if (amount <= 0) return 0;
  if (amount > maxOperationAmount) return 0;
  if (amount > settings.maxBalance) return 0;

  long available = players[from].balance;
  if (available <= 0) return 0;

  // 1. Считаем количество активных получателей
  int count = 0;
  for (int i = 0; i < selectedPlayerCount; i++) {
    if (i == from) continue;
    if (players[i].eliminated) continue;
    if (settings.autoEndGame && players[i].balance == 0) continue;
    count++;
  }
  if (count == 0) return 0;

  // 2. Считаем сумму на игрока
  long perPlayer = available / count;
  long remainder = available % count;

  if (perPlayer <= 0) return 0;  // даже по 1 не можем дать

  // 3. Списываем всё у отправителя
  players[from].balance = 0;
  playerEliminated(from);

  // 4. Начисляем только активным игрокам
  for (int i = 0; i < selectedPlayerCount; i++) {
    if (i == from) continue;
    if (players[i].eliminated) continue;
    if (settings.autoEndGame && players[i].balance == 0) continue;
    long allowed = settings.maxBalance - players[i].balance;
    long give = perPlayer;
    if (give > allowed) give = allowed;

    players[i].balance += give;
  }
  long realTotal = perPlayer * count;
  checkAutoEndGame();
  return realTotal;
}

long doPayBank(int from, long amount) {
  if (amount <= 0) return 0;

  // Лимит максимальной суммы
  if (amount > settings.maxBalance) return 0;
  if (amount > maxOperationAmount) return 0;

  // Если денег не хватает — платим всё, что есть
  if (players[from].balance < amount) {
    amount = players[from].balance;
    players[from].balance = 0;
    playerEliminated(from);
  } else {
    players[from].balance -= amount;
  }

  if (checkAutoEndGame()) {
    return amount;
  }
  return amount;  // ← реальная сумма
}

long doGetBank(int to, long amount) {
  if (amount <= 0) return 0;
  if (amount > settings.maxBalance) return 0;  // Лимит максимальной суммы
  if (amount > maxOperationAmount) return 0;
  long allowed = settings.maxBalance - players[to].balance;
  if (allowed <= 0) return 0;
  if (amount > allowed) amount = allowed;
  players[to].balance += amount;
  if (checkAutoEndGame()) {
    return amount;
  }
  return amount;  // ← реальная сумма
}

// ---------------------------------------------------------
// СОХРАНЕНИЕ СОСТОЯНИЯ ИГРЫ В LittleFS
// ---------------------------------------------------------
void saveGameState() {
  File f = LittleFS.open("/game.dat", "w");
  if (!f) return;
  f.write((byte*)&selectedPlayerCount, sizeof(selectedPlayerCount));  // Количество игроков
  uint32_t ts = gameStartTime.unixtime();                             // Время начала игры (Unix time)
  f.write((byte*)&ts, sizeof(ts));

  // Игроки
  for (int i = 0; i < selectedPlayerCount; i++) {
    f.write(players[i].uid, 4);
    f.write((byte*)&players[i].balance, sizeof(long));
  }
  f.close();
}

// ---------------------------------------------------------
// ЗАГРУЗКА СОСТОЯНИЯ ИГРЫ ИЗ LittleFS
// ---------------------------------------------------------
void loadGameState() {
  File f = LittleFS.open("/game.dat", "r");
  if (!f || f.size() == 0) {
    gameActive = false;
    return;
  }
  f.read((byte*)&selectedPlayerCount, sizeof(selectedPlayerCount));  // Количество игроков
  // Время начала игры
  uint32_t ts;
  f.read((byte*)&ts, sizeof(ts));
  gameStartTime = DateTime(ts);
  // Игроки
  for (int i = 0; i < selectedPlayerCount; i++) {
    f.read(players[i].uid, 4);
    f.read((byte*)&players[i].balance, sizeof(long));
    // Имя генерируем автоматически
    static char nameBuf[16];
    snprintf(nameBuf, sizeof(nameBuf), "Игрок %d", i + 1);
    // players[i].name = strdup(nameBuf);
    static char names[8][16];
    snprintf(names[i], sizeof(names[i]), "Игрок %d", i + 1);
    players[i].name = names[i];
  }
  f.close();
  gameActive = true;
}

// сохранение настроек игры
void settingsLoad() {
  EEPROM.get(0, settings);
  if (settings.magic != SETTINGS_MAGIC || settings.version != SETTINGS_VERSION) {
    Serial.println("EEPROM INVALID → loading defaults");
    // Игровые настройки
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    settings.maxBalance = 30004;
    settings.confirmLargeOps = true;
    settings.largeOpThreshold = 5003;
    settings.autoEndGame = true;

    // Системные настройки
    settings.currency = 0;
    settings.wifiEnabled = false;
    settingsSave();
  } else {
    Serial.println("EEPROM OK → using saved settings");
  }
}

void settingsSave() {
  Serial.println("Clear ... ");
  // очистка EEPROM
  for (int i = 0; i < sizeof(Settings) + 1; i++) EEPROM.write(i, 0xFF);
  EEPROM.commit();

  // Запись текущих настроек
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  EEPROM.put(0, settings);
  EEPROM.commit();
  Serial.println("Saved maxBalance = " + String(settings.maxBalance));
}
