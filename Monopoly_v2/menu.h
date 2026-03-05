#pragma once
#include <Arduino.h>

enum MenuState {
  // Стартовые экраны
  STATE_SPLASH,
  STATE_MAIN_MENU,
  MENU_HELP,
  // Мастер новой игры
  STATE_NEWGAME_NUMPLAYERS,
  STATE_NEWGAME_REGCARD,
  STATE_NEWGAME_STARTBALANCE,
  STATE_NEWGAME_CONFIRM,

  // Игровой режим
  STATE_GAME_WAITCARD,
  STATE_GAME_PLAYERMENU,
  STATE_GAME_TRANSFER_SELECTPLAYER,
  STATE_GAME_TRANSFER_SUM,
  STATE_GAME_BANK_PAY,
  STATE_GAME_BANK_GET,
  STATE_GAME_RESULT,
  STATE_GAME_UNDO,
  STATE_GAME_BALANCES,
  STATE_GAME_SUM_PRESET,
  STATE_GAME_SUM_MANUAL,
  STATE_GAME_CONFIRM_LARGE,
  // режим настроек
  STATE_SETTINGS_MENU,
  STATE_SETTINGS_WIFI,
  STATE_SETTINGS_GAME,
  STATE_SETTINGS_SYSTEM,
  STATE_SETTINGS_GAME_EDIT_MAXBALANCE,
  STATE_SETTINGS_GAME_EDIT_THRESHOLD


};

extern MenuState menuState;
extern int activePlayerIndex;
extern MenuState returnFromBalances;
extern MenuState amountNextState;
extern long presetAmounts[5];
extern bool rtc_ok;
extern bool gameActive;

extern int currentRegisteringPlayer;
extern int selectedStartBalance;

extern int transferTargetIndex;
extern long transferAmount;


// Инициализация меню
void menuInit();
void i2cBusRecovery();
// Обновление (таймеры, время игры, RTC)
void menuUpdate();

// Обработка клавиш
void menuHandleKey(String key);

// Переход в состояние
void setMenuState(MenuState newState);
void handleCardInNewGameWizard(byte uid[4]);

struct HelpPage {
  const char* l1;
  const char* l2;
  const char* l3;
  const char* l4;
  const char* l5;
};


extern HelpPage helpPages[];
extern int helpPageCount;
extern int helpPageIndex;
extern bool helpMode;

// меню настроек 
void settingsInit();
void settingsUpdate();
void settingsHandleKey(String key);

