#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include "players.h"
#include "menu.h"
// Инициализация дисплея
extern U8G2_ST7920_128X64_F_SW_SPI u8g2;
void uiInit();

// Универсальная функция отрисовки 5 строк
void drawScreen(const char* line1,
                const char* line2 = "",
                const char* line3 = "",
                const char* line4 = "",
                const char* line5 = "");
void drawHelpScreen();

void uiShowSettingsMenu();
void uiShowSettingsGame();
void uiShowSettingsSystem();
void uiShowRTCTest() ;
void uiShowBatteryTest();
void uiShowServiceMenu();
void uiShowSettingsCurrency();
void uiShowSettingsSysInfo();
extern RTC_DS1307 rtc;
// Отрисовка заголовка игрового режима (время игры + текущее время)
void drawGameHeader(DateTime now, uint32_t gameSeconds);

void showSplash();
// --- Экраны главного меню ---
// void uiShowSplash();
void uiShowMainMenu(bool hasSave);
void drawHelpQR();
// --- Мастер новой игры ---
void uiShowNewGame_NumPlayers(int count);
void uiShowNewGame_RegCard(int playerIndex);
void uiShowNewGame_RegCardOK(int playerIndex, byte uid[4]);
void uiShowNewGame_RegCardError();
void uiShowNewGame_StartBalance(int balance);
void uiShowNewGame_Confirm(int players, int balance);

// --- Игровой режим ---
void uiShowGame_WaitCard(DateTime now, uint32_t gameSeconds);
void uiShowGame_PlayerMenu(const char* name, int balance);
void uiShowGame_SelectPlayer();
void uiShowGame_EnterAmount(long amount);
void uiShowGame_Result(const char* line1, const char* line2, const char* line3);

void uiShowGame_Balances(int count, int balances[]);
void uiShowPlayerFinish(const char* name);
void showBankruptFlash(int idx);
// --- Новый ввод суммы ---
void uiShowGame_SumPreset();
void uiShowGame_SumManual(long amount);

void uiShowEditMaxBalance(long value);
void uiShowEditThreshold(long value);
void uiShowConfirmLarge(long amount);
