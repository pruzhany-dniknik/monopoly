// ui.cpp
#include "ui.h"
#include "qr_bitmap.h"
#include "input.h"

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 14, 13, 15, 0); // D1 Wemos UNO

// ---------------------------------------------------------
// ИНИЦИАЛИЗАЦИЯ
// ---------------------------------------------------------
void uiInit() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_t_cyrillic);
}

void drawScreen(const char* line1,
                const char* line2,
                const char* line3,
                const char* line4,
                const char* line5) {

  if (!line1) line1 = "";
  if (!line2) line2 = "";
  if (!line3) line3 = "";
  if (!line4) line4 = "";
  if (!line5) line5 = "";
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);  // внешняя рамка
  u8g2.drawBox(0, 0, 128, 12);    // заголовок внутри рамки
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(4, 10, line1);
  u8g2.setDrawColor(1);
  // --- Контент ---
  u8g2.drawUTF8(4, 24, line2);
  u8g2.drawUTF8(4, 36, line3);
  u8g2.drawUTF8(4, 48, line4);
  u8g2.drawUTF8(4, 60, line5);

  u8g2.sendBuffer();
}

// ---------------------------------------------------------
// СПРАВКА
// ---------------------------------------------------------
void drawHelpScreen() {
  if (helpPageIndex == helpPageCount - 1) {
    drawHelpQR();
    return;
  }
  HelpPage& p = helpPages[helpPageIndex];
  u8g2.clearBuffer();
  char title[48];
  snprintf(title, sizeof(title),
           "СПРАВКА  %d из %d",
           helpPageIndex + 1,
           helpPageCount);

  u8g2.drawBox(0, 0, 128, 12);
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(4, 10, title);
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawUTF8(2, 22, p.l1);
  u8g2.drawUTF8(2, 32, p.l2);
  u8g2.drawUTF8(2, 42, p.l3);
  u8g2.drawUTF8(2, 52, p.l4);
  u8g2.drawUTF8(2, 62, p.l5);
  u8g2.setFont(u8g2_font_6x12_t_cyrillic);

  u8g2.sendBuffer();
}

void drawHelpQR() {
  const int scale = 2;
  const int sizeX = qrWidth * scale;
  const int sizeY = qrHeight * scale;
  const int ox = 128 - sizeX;       // QR справа
  const int oy = (64 - sizeY) / 2;  // центр по вертикали
  u8g2.clearBuffer();

  // Заголовок
  u8g2.drawBox(0, 0, 60, 12);
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(12, 10, "GITHUB");
  u8g2.drawBox(ox - 2, oy - 2, sizeX + 4, sizeY + 4);
  u8g2.setDrawColor(1);

  // Текст слева
  u8g2.drawUTF8(4, 24, "Info");
  u8g2.drawUTF8(4, 34, "Update");
  u8g2.drawUTF8(4, 44, "Help");
  u8g2.drawUTF8(4, 54, "Feedback");

  // Рисуем QR-код
  for (int y = 0; y < qrHeight; y++) {
    for (int x = 0; x < qrWidth; x++) {
      int byteIndex = (y * ((qrWidth + 7) / 8)) + (x >> 3);
      uint8_t b = pgm_read_byte(&qrBitmap[byteIndex]);
      bool bit = b & (0x80 >> (x & 7));
      if (!bit) {
        u8g2.drawBox(ox + x * scale, oy + y * scale, scale, scale);
      }
    }
  }
  u8g2.drawFrame(64, 2, 62, 62);
  u8g2.sendBuffer();
}

// ---------------------------------------------------------
// ЗАСТАВКА
// ---------------------------------------------------------
void showSplash() {
  drawScreen("$$$  МОНОПОЛИЯ  $$$",
             "Банковский терминал",
             "  DNikNik Edition",
             "--------------------",
             "   ver.2.2 beta");
  delay(2000);
}

// ---------------------------------------------------------
// ГЛАВНОЕ МЕНЮ
// ---------------------------------------------------------
void uiShowMainMenu(bool hasSave) {
  drawScreen("ГЛАВНОЕ МЕНЮ",
             "[1] Начать игру",
             hasSave ? "[2] Продолжить игру" : "    нет сохранения...",
             "[3] Настройки",
             "[4] Справка");
}

// ---------------------------------------------------------
// МЕНЮ НАСТРОЕК
// ---------------------------------------------------------
void uiShowSettingsMenu() {
  drawScreen(
    "НАСТРОЙКИ",
    "[1] Игра",
    "[2] Система",
    "[3] Сервис",
    "[#] Сохранить");
}

void uiShowServiceMenu() {
  drawScreen(
    "SERVICE",
    "[1] RTC",
    "[2] Power",
    "[3] WiFi",
    "[Esc] Back");
}

void uiShowSettingsGame() {
  char line1[32];
  char line2[32];
  char line3[32];
  char line4[32];

  snprintf(line1, sizeof(line1), "[1] Баланс: %ld%s", settings.maxBalance, names[settings.currency]);
  snprintf(line2, sizeof(line2), "[2] Подтв.: %s", settings.confirmLargeOps ? "ВКЛ" : "ВЫКЛ");
  snprintf(line3, sizeof(line3), "[3] Порог:  %ld%s", settings.largeOpThreshold, names[settings.currency]);
  snprintf(line4, sizeof(line4), "[4] Победа: %s", settings.autoEndGame ? "ВКЛ" : "ВЫКЛ");

  drawScreen(
    "ИГРОВЫЕ НАСТРОЙКИ",
    line1,
    line2,
    line3,
    line4);
}

// СИСТЕМНЫЕ НАСТРОЙКИ
void uiShowSettingsSystem() {
  drawScreen(
    "СИСТЕМНЫЕ НАСТРОЙКИ",
    "[1] Валюта",
    "[2] ...",
    "[3] ...",
    "[4] Информация");
}

void uiShowSettingsCurrency() {
  char line2[64];
  snprintf(line2, sizeof(line2), "Текущая: %s", currencyNames[settings.currency]);
  drawScreen(
    "ВАЛЮТА",
    line2,
    "[1] -    [3] Е",
    "[2] $    [4] р",
    "[Ent] Ok");
}

void uiShowSettingsSysInfo() {
  char l1[32], l2[32], l3[32], l4[32];
  snprintf(l1, sizeof(l1), "FW: %s", FW_VERSION);
  snprintf(l2, sizeof(l2), "Settings: v%d", settings.version);
  snprintf(l3, sizeof(l3), "Flash: %u KB", ESP.getFlashChipSize() / 1024);
  snprintf(l4, sizeof(l4), "Free RAM: %u", ESP.getFreeHeap());
  drawScreen("ИНФОРМАЦИЯ О СИСТЕМЕ", l1, l2, l3, l4);
}

// ---------------------------------------------------------
// МАСТЕР НОВОЙ ИГРЫ
// ---------------------------------------------------------
void uiShowNewGame_NumPlayers(int count) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Игроков: %d", count);

  drawScreen("НОВАЯ ИГРА",
             line2,
             "<> изменить",
             "[Ent] далее",
             "[Esc] назад");
}

void uiShowNewGame_RegCard(int playerIndex) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Игрок %d", playerIndex + 1);

  drawScreen("РЕГИСТРАЦИЯ",
             line2,
             "Приложите карту",
             "",
             "[Esc] назад");
}

void uiShowNewGame_RegCardOK(int playerIndex, byte uid[4]) {
  char line2[64];
  char line3[64];

  snprintf(line2, sizeof(line2), "Игрок %d", playerIndex + 1);
  snprintf(line3, sizeof(line3), "UID: %02X %02X %02X %02X",
           uid[0], uid[1], uid[2], uid[3]);
  drawScreen("КАРТА ПРИНЯТА",
             line2,
             line3,
             "",
             "[Ent] далее");
}

void uiShowNewGame_RegCardError() {
  drawScreen("ОШИБКА",
             "Карта уже есть",
             "Ищите другую",
             "",
             "[Esc] назад");
}

void uiShowNewGame_StartBalance(int balance) {
  char line2[64];
  snprintf(line2, sizeof(line2), "[ %d ]", balance);
  drawScreen("СТАРТ КАПИТАЛ",
             line2,
             "<> +-500",
             "[Ent] далее",
             "[Esc] назад");
}

void uiShowNewGame_Confirm(int players, int balance) {
  char line2[64];
  char line3[64];
  snprintf(line2, sizeof(line2), "Игроков: %d", players);
  snprintf(line3, sizeof(line3), "Капитал: %d", balance);
  drawScreen("ПОДТВЕРЖДЕНИЕ",
             line2,
             line3,
             "[*] начать",
             "[#] назад");
}

// ---------------------------------------------------------
// ИГРОВОЙ РЕЖИМ
// ---------------------------------------------------------

void uiShowGame_WaitCard(DateTime now, uint32_t gameSeconds) {
  char nowStr[16];
  snprintf(nowStr, sizeof(nowStr), "%02d:%02d", now.hour(), now.minute());
  char header[64];
  snprintf(header, sizeof(header), "МОНОПОЛИЯ      %s", nowStr);
  uint32_t h = gameSeconds / 3600;
  uint32_t m = (gameSeconds / 60) % 60;
  uint32_t s = gameSeconds % 60;
  char gameTime[32];
  snprintf(gameTime, sizeof(gameTime),
           "Время игры: %02lu:%02lu:%02lu", h, m, s);
  static uint32_t lastAnim = 0;
  static int dots = 0;
  if (millis() - lastAnim > 500) {   // скорость анимации
    dots = (dots + 1) % 4;           // 0,1,2,3
    lastAnim = millis();
  }
  char waitStr[32];
  snprintf(waitStr, sizeof(waitStr), "Ожидание карты%.*s", dots, "...");
  const char* empty = "____________________";
  if (millis() - lastBatteryCheck > eventTimeout) {
    float v = readBatteryVoltage();
    cachedBatteryPercent = batteryPercent(v);
    lastBatteryCheck = millis();
  }
  const char* left = "[#] Меню ";
  char right[16];
  snprintf(right, sizeof(right), "[%d%%]", cachedBatteryPercent);
  const int width = 21;  // ширина строки в символах
  int spaces = width - strlen(left) - strlen(right);
  if (spaces < 1) spaces = 1;
  char bottom[32];
  snprintf(bottom, sizeof(bottom), "%s%*s%s", left, spaces, "", right);
  drawScreen(
    header,
    gameTime,
    waitStr,
    empty,
    bottom
  );
}

void uiShowGame_PlayerMenu(const char* name, int balance) {
  char header[64];
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  // snprintf(header, sizeof(header), "%s : %d", name ? name : "Игрок", balance, names[settings.currency]);
  snprintf(header, sizeof(header), "%s : %d%s",
           name ? name : "Игрок",
           balance,
           names[settings.currency]);
  drawScreen(header,
             "[1] Перевод игроку",
             "[2] К оплате (банк)",
             "[3] Получить (банк)",
             "[4] Завершить игру");
}

// ---------------------------------------------------------
// ЭКРАН ВЫБОРА ИГРОКА
// ---------------------------------------------------------
void uiShowGame_SelectPlayer() {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  char line3[64];
  char line4[64];
  line3[0] = 0;
  line4[0] = 0;
  for (int i = 1; i <= selectedPlayerCount && i <= 4; i++) {
    snprintf(line3 + strlen(line3),
             sizeof(line3) - strlen(line3),
             "[%d] ", i);
  }
  if (selectedPlayerCount > 4) {
    for (int i = 5; i <= selectedPlayerCount && i <= 8; i++) {
      snprintf(line4 + strlen(line4),
               sizeof(line4) - strlen(line4),
               "[%d] ", i);
    }
  }

  drawScreen("ПЕРЕВОД",
             "Выберите игрока:",
             line3,
             line4,
             "[*] всем игрокам");
}

// ---------------------------------------------------------
//  ФИКСИРОВАННЫЕ СУММЫ
// ---------------------------------------------------------
void uiShowGame_SumPreset() {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  char l2[32];
  char l3[32];
  char l4[32];
  snprintf(l2, sizeof(l2), "[1] %ld%s  [4] %ld%s",
           presetAmounts[0], names[settings.currency],
           presetAmounts[3], names[settings.currency]);

  snprintf(l3, sizeof(l3), "[2] %ld%s  [5] %ld%s",
           presetAmounts[1], names[settings.currency],
           presetAmounts[4], names[settings.currency]);

  snprintf(l4, sizeof(l4), "[3] %ld%s  [*] Другая",
           presetAmounts[2], names[settings.currency]);
  drawScreen(" ВВОД СУММЫ", l2, l3, l4, "[Ent]OK   [Esc]Назад");
}

// ---------------------------------------------------------
//  РУЧНОЙ ВВОД
// ---------------------------------------------------------
void uiShowGame_SumManual(long amount) {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld", amount);

  drawScreen("РУЧНОЙ ВВОД СУММЫ", line2, "Введите сумму",
    "[Ent] OK","[Esc] Назад");
}

// ---------------------------------------------------------
//  ПОДТВЕРЖДЕНИЯ СУММЫ
// ---------------------------------------------------------
void uiShowGame_EnterAmount(long amount) {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld%s", amount, names[settings.currency]);

  drawScreen("ВВОД СУММЫ", line2, "",
             "[Ent] OK", "[Esc] Отмена");
}

void uiShowConfirmLarge(long amount) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld%s", amount, names[settings.currency]);

  drawScreen("КРУПНАЯ ОПЕРАЦИЯ",
             line2,
             "",
             "[Ent] Подтвердить",
             "[Esc] Отмена");
}

// ---------------------------------------------------------
// РЕЗУЛЬТАТЫ
// ---------------------------------------------------------
void uiShowGame_Result(const char* line1, const char* line2, const char* line3) {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  drawScreen("РЕЗУЛЬТАТ",
             line1,
             line2,
             line3,
             "[Ent] OK");
}

// void uiShowGame_Balances(int count, int balances[]) {
//   char l2[64] = "";
//   char l3[64] = "";
//   char l4[64] = "";
//   char l5[64] = "";
//   char* lines[4] = { l2, l3, l4, l5 };

//   const int colWidth = 10;  // ширина первой колонки (можно 10–14)
//   int lineIndex = 0;

//   for (int i = 0; i < count; i += 2) {
//     if (lineIndex >= 4) break;

//     char left[32], right[32];

//  // Левая колонка
//     if (balances[i] == 0 && settings.autoEndGame) {
//         snprintf(left, sizeof(left), "%d:%s", i + 1, "выбыл");
//     } else {
//         snprintf(left, sizeof(left), "%d:%d%s",
//                 i + 1,
//                 balances[i],
//                 names[settings.currency]);   // валюта
//     }

//     // Правая колонка
//     if (i + 1 < count) {
//         if (balances[i + 1] == 0 && settings.autoEndGame) {
//             snprintf(right, sizeof(right), "%d:%s", i + 2, "выбыл");
//         } else {
//             snprintf(right, sizeof(right), "%d:%d%s",
//                     i + 2,
//                     balances[i + 1],
//                     names[settings.currency]);   // валюта
//         }

//         snprintf(lines[lineIndex], 64, "%-*s %s", colWidth, left, right);
//     } else {
//         snprintf(lines[lineIndex], 64, "%s", left);
//     }

//     lineIndex++;
//   }
//   drawScreen("БАЛАНСЫ ИГРОКОВ:", l2, l3, l4, l5);
// }
void uiShowGame_Balances(int count, int balances[]) {
  static uint32_t lastDraw = 0;
if (millis() - lastDraw < 150) return;  // ~6 FPS
lastDraw = millis();

  const int leftX = 4;    // позиция левой колонки
  const int rightX = 70;  // позиция правой колонки
  const int startY = 24;  // первая строка
  const int stepY = 12;   // шаг между строками

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.drawBox(0, 0, 128, 12);
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(4, 10, "БАЛАНСЫ ИГРОКОВ");
  u8g2.setDrawColor(1);

  int y = startY;

  for (int i = 0; i < count; i += 2) {
    if (y > 60) break;  // максимум 4 строки
    char left[32];
    char right[32];
    // Левая колонка
    if (balances[i] == 0 && settings.autoEndGame) {
      snprintf(left, sizeof(left), "%d:%s", i + 1, "БАНКРОТ");
    } else {
      snprintf(left, sizeof(left), "%d:%d%s",
               i + 1,
               balances[i],
               names[settings.currency]);
    }
    // Правая колонка (если игрок существует)
    if (i + 1 < count) {
      if (balances[i + 1] == 0 && settings.autoEndGame) {
        snprintf(right, sizeof(right), "%d:%s", i + 2, "БАНКРОТ");
      } else {
        snprintf(right, sizeof(right), "%d:%d%s",
                 i + 2,
                 balances[i + 1],
                 names[settings.currency]);
      }
    } else {
      right[0] = 0;  // пусто
    }
    // Рисуем две колонки независимо
    u8g2.drawUTF8(leftX, y, left);
    if (right[0]) {
      u8g2.drawUTF8(rightX, y, right);
    }
    y += stepY;
  }
  u8g2.sendBuffer();
}


void uiShowEditMaxBalance(long value) {
  char line3[32];
  snprintf(line3, sizeof(line3), ">> %ld", value);
  drawScreen(
    "ИЗМЕНЕНИЕ ЗНАЧЕНИЯ",
    "Максимальный порог:",
    line3,
    "[Ent] OK",
    "[Esc] Отмена");
}

void uiShowEditThreshold(long value) {
  char line3[32];
  snprintf(line3, sizeof(line3), ">> %ld", value);
  drawScreen(
    "ИЗМЕНЕНИЕ ЗНАЧЕНИЯ",
    "Порог операции:",
    line3,
    "[Ent] OK",
    "[Esc] Отмена");
}

void uiShowPlayerFinish(const char* name) {
  char l2[32];
  snprintf(l2, sizeof(l2), "%s", name);
  drawScreen(
    "ЗАВЕРШИТЬ ИГРУ",
    l2,
    "станет банкротом",
    "[Ent] ОК",
    "[Esc] Отмена");
}

DateTime safeNow();
uint32_t getGameSeconds();

void uiShowGame_WaitCardSimple() {
  DateTime now = safeNow();         // safeNow() уже есть в menu.cpp, объявим extern
  uint32_t sec = getGameSeconds();  // getGameSeconds() — глобальная функция
  uiShowGame_WaitCard(now, sec);
}

void showBankruptFlash(int idx) {
  const char* text = "$$ БАНКРОТ $$";
  u8g2.setFont(u8g2_font_6x12_t_cyrillic);
  for (int i = 0; i < 4; i++) {
    u8g2.drawUTF8(4, 47, text);
    u8g2.sendBuffer();
    delay(50);
    u8g2.setDrawColor(0);
    u8g2.drawBox(3, 36, 120, 13);
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
    delay(50);
  }
  uiShowGame_WaitCardSimple();
}

void uiShowRTCTest() {
  char l2[32], l3[32], l4[64], l5[32];
  l2[0] = l3[0] = l4[0] = l5[0] = 0;

  // --- 1. Проверяем ACK от DS1307 ---
  Wire.beginTransmission(0x68);
  bool rtc_ack = (Wire.endTransmission() == 0);
  snprintf(l2, sizeof(l2), "RTC ACK: %s", rtc_ack ? "OK" : "FAIL");

  // --- 2. Проверяем rtc.begin() ---
  bool begin_ok = rtc.begin();
  snprintf(l3, sizeof(l3), "BEGIN: %s", begin_ok ? "OK" : "FAIL");

  // --- 3. Сканируем I2C ---
  char devices[64] = "";
  bool found = false;

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found = true;
      char buf[8];
      snprintf(buf, sizeof(buf), "0x%02X ", addr);
      strcat(devices, buf);
    }
  }

  if (!found) {
    snprintf(l4, sizeof(l4), "I2C: нет устройств");
  } else {
    snprintf(l4, sizeof(l4), "I2C: %s", devices);
  }

  // --- 4. Если RTC жив — показываем время ---
  if (rtc_ack && begin_ok) {
    DateTime now = rtc.now();
    snprintf(l5, sizeof(l5), "%02d:%02d:%02d  %02d.%02d.%04d",
             now.hour(), now.minute(), now.second(),
             now.day(), now.month(), now.year());
  } else {
    snprintf(l5, sizeof(l5), "Время: ---");
  }
  drawScreen("RTC ТЕСТ", l2, l3, l4, l5);
}

void uiShowBatteryTest() {
  float v = readBatteryVoltage();
  int p = batteryPercent(v);
  char l2[32], l3[32];
  snprintf(l2, sizeof(l2), "Напряжение: %.2f В", v);
  snprintf(l3, sizeof(l3), "Заряд: %d%%", p);
  drawScreen("18650 ТЕСТ", l2, l3, "", "");
}
