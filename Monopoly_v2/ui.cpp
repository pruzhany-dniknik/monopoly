#include "ui.h"
#include "qr_bitmap.h"

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 14, 13, 15, 0);

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
  int scale = 2;                 // 2 пикселя на модуль
  int sizeX = qrWidth * scale;   // ширина QR
  int sizeY = qrHeight * scale;  // высота QR

  int ox = 128 - sizeX - 2;       // QR справа, отступ 2 px
  int oy = (64 - sizeY) / 2 + 2;  // центр по вертикали

  u8g2.clearBuffer();

  // Укороченная верхняя полоска (только над текстом слева)
  u8g2.drawBox(0, 0, 65, 12);  // 80 px ширина, чтобы не перекрывать QR
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(4, 10, "ТЕЛЕГРАМ");
  u8g2.setDrawColor(1);

  // Текст слева (пример)
  u8g2.drawUTF8(4, 24, "@LITTLE");
  u8g2.drawUTF8(4, 34, "инфо");
  u8g2.drawUTF8(4, 44, "патчи");
  u8g2.drawUTF8(4, 54, "фидбэк");

  // Рисуем QR-код справа
  for (int y = 0; y < qrHeight; y++) {
    for (int x = 0; x < qrWidth; x++) {
      int byteIndex = (y * ((qrWidth + 7) / 8)) + (x >> 3);
      uint8_t b = pgm_read_byte(&qrBitmap[byteIndex]);
      if (b & (0x80 >> (x & 7))) {
        u8g2.drawBox(ox + x * scale, oy + y * scale, scale, scale);
      }
    }
  }

  u8g2.sendBuffer();
}


// ---------------------------------------------------------
// ЗАСТАВКА
// ---------------------------------------------------------
void showSplash() {
  drawScreen("$$$  МОНОПОЛИЯ  $$$",
             "  Ардуино-терминал",
             "     by DNikNik",
             "--------------------",
             "ver.2.08 beta");
  delay(4000);
}

// ---------------------------------------------------------
// ГЛАВНОЕ МЕНЮ
// ---------------------------------------------------------
void uiShowSplash() {
  u8g2.clearBuffer();

  // Рамка
  u8g2.drawFrame(0, 0, 128, 64);
  // Заголовок
  u8g2.drawBox(0, 0, 128, 12);
  u8g2.setDrawColor(0);
  u8g2.drawUTF8(4, 11, "ТЕРМИНАЛ ИГРЫ");
  u8g2.setDrawColor(1);
  // Текст
  u8g2.drawUTF8(4, 30, "Загрузка...");
  u8g2.sendBuffer();
  // Прогресс-бар
  for (int i = 0; i <= 100; i += 10) {
    u8g2.drawFrame(10, 45, 108, 10);           // рамка бара
    u8g2.drawBox(12, 47, (104 * i) / 100, 6);  // заполнение
    u8g2.sendBuffer();
    delay(2);
  }
}

void uiShowMainMenu(bool hasSave) {
  drawScreen("ГЛАВНОЕ МЕНЮ",
             "[1] Начать игру",
             hasSave ? "[2] Продолжить игру" : "   (нет сохранения)",
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
    "[3] WiFi",
    "[Esc] Назад");
}

void uiShowSettingsGame() {
  char line1[32];
  char line2[32];
  char line3[32];
  char line4[32];

  snprintf(line1, sizeof(line1), "[1] Баланс: %ld", settings.maxBalance);
  snprintf(line2, sizeof(line2), "[2] Подтв.: %s", settings.confirmLargeOps ? "ВКЛ" : "ВЫКЛ");
  snprintf(line3, sizeof(line3), "[3] Порог:  %ld", settings.largeOpThreshold);
  snprintf(line4, sizeof(line4), "[4] Победа: %s", settings.autoEndGame ? "ВКЛ" : "ВЫКЛ");

  drawScreen(
    "ИГРОВЫЕ НАСТРОЙКИ",
    line1,
    line2,
    line3,
    line4);
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
             "Приложите другую",
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
  char header[64];

  uint32_t minutes = gameSeconds / 60;
  uint32_t hours = minutes / 60;

  char gameTime[16];
  if (hours == 0)
    snprintf(gameTime, sizeof(gameTime), "%02lu:%02lu", minutes, gameSeconds % 60);
  else
    snprintf(gameTime, sizeof(gameTime), "%02lu:%02lu", hours, minutes % 60);

  char nowStr[16];
  snprintf(nowStr, sizeof(nowStr), "%02d:%02d", now.hour(), now.minute());

  snprintf(header, sizeof(header), "ИГРА: %s    %s", gameTime, nowStr);

  drawScreen(header,
             "Приложите карту...",
             "",
             "[Esc]отмена операции",
             "[#] выход в меню");
}

void uiShowGame_PlayerMenu(const char* name, int balance) {
  char header[64];
  snprintf(header, sizeof(header), "%s : %d", name ? name : "Игрок", balance);

  drawScreen(header,
             "[1] Перевод игроку",
             "[2] К оплате (банк)",
             "[3] Получить (банк)",
             "");
}

// ---------------------------------------------------------
// ЭКРАН ВЫБОРА ИГРОКА
// ---------------------------------------------------------
void uiShowGame_SelectPlayer() {
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
             "");
}

// ---------------------------------------------------------
//  ФИКСИРОВАННЫЕ СУММЫ
// ---------------------------------------------------------
void uiShowGame_SumPreset() {
  char l2[32];
  char l3[32];
  char l4[32];
  snprintf(l2, sizeof(l2), "[1] %ld   [4] %ld", presetAmounts[0], presetAmounts[3]);
  snprintf(l3, sizeof(l3), "[2] %ld   [5] %ld", presetAmounts[1], presetAmounts[4]);
  snprintf(l4, sizeof(l4), "[3] %ld   [*] Другая", presetAmounts[2]);
  drawScreen(
    " ВВОД СУММЫ",
    l2,
    l3,
    l4,
    "[Ent]OK   [Esc]Назад");
}


// ---------------------------------------------------------
//  РУЧНОЙ ВВОД
// ---------------------------------------------------------
void uiShowGame_SumManual(long amount) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld", amount);

  drawScreen(
    "РУЧНОЙ ВВОД СУММЫ",
    line2,
    "Введите сумму",
    "[Ent] OK",
    "[Esc] Назад");
}

// ---------------------------------------------------------
//  ПОДТВЕРЖДЕНИЯ СУММЫ
// ---------------------------------------------------------
void uiShowGame_EnterAmount(long amount) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld", amount);

  drawScreen("ВВОД СУММЫ",
             line2,
             "",
             "[Ent] OK",
             "[Esc] Отмена");
}

void uiShowConfirmLarge(long amount) {
  char line2[64];
  snprintf(line2, sizeof(line2), "Сумма: %ld", amount);

  drawScreen("КРУПНАЯ ОПЕРАЦИЯ",
             line2,
             "",
             "[Ent] Подтвердить",
             "[Esc] Отмена");
}

// ---------------------------------------------------------
// РЕЗУЛЬТАТЫ
// ---------------------------------------------------------
void uiShowGame_Result(const char* line2, const char* line3) {
  drawScreen("ОПЕРАЦИЯ",
             line2,
             line3,
             "",
             "[Ent] далее");
}

void uiShowGame_Undo_NoAction() {
  drawScreen("ОТМЕНА",
             "Нет действий",
             "",
             "",
             "[Esc] назад");
}

void uiShowGame_Undo_Action(const char* line2, const char* line3) {
  drawScreen("ОТМЕНА",
             line2,
             line3,
             "[*] подтвердить",
             "[#] назад");
}

void uiShowGame_Balances(int count, int balances[]) {
  char l2[64] = "";
  char l3[64] = "";
  char l4[64] = "";
  char l5[64] = "";

  char* lines[4] = { l2, l3, l4, l5 };

  int lineIndex = 0;

  for (int i = 0; i < count; i += 2) {
    if (lineIndex >= 4) break;

    if (i + 1 < count)
      snprintf(lines[lineIndex], 64, "%d:%d   %d:%d",
               i + 1, balances[i],
               i + 2, balances[i + 1]);
    else
      snprintf(lines[lineIndex], 64, "%d:%d",
               i + 1, balances[i]);

    lineIndex++;
  }

  drawScreen("БАЛАНСЫ ИГРОКОВ:", l2, l3, l4, l5);
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
