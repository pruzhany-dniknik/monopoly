// menu_keys.cpp
#include "menu.h"
#include "ui.h"
#include "players.h"
#include <Arduino.h>


extern MenuState menuState;
extern MenuState returnFromBalances;
extern MenuState amountNextState;

extern int currentRegisteringPlayer;
extern int selectedStartBalance;

extern int activePlayerIndex;
extern int transferTargetIndex;
extern long transferAmount;

extern int selectedPlayerCount;
extern bool helpMode;
extern int helpPageIndex;
extern int helpPageCount;

extern RTC_DS1307 rtc;
extern DateTime gameStartTime;
extern bool gameActive;

// extern LastAction lastAction;

// extern long clampAmount(long value);
extern long presetAmounts[5];

long tempValue = 0;

// ---------------------------------------------------------
// ОБРАБОТКА КЛАВИШ
// ---------------------------------------------------------
void menuHandleKey(String key) {

  // --- Глобальная клавиша F2 (H) — только в игровом режиме ---
  if (key == "H") {
    if (menuState == STATE_GAME_WAITCARD || menuState == STATE_GAME_PLAYERMENU) {
      returnFromBalances = menuState;
      setMenuState(STATE_GAME_BALANCES);
      return;
    }
  }

  // --- Режим справки ---
  if (helpMode) {
    if (key == "L") {
      if (helpPageIndex > 0) helpPageIndex--;
      drawHelpScreen();
    } else if (key == "R") {
      if (helpPageIndex < helpPageCount - 1) helpPageIndex++;
      drawHelpScreen();
    } else if (key == "E" || key == "#") {
      helpMode = false;
      setMenuState(STATE_MAIN_MENU);
    }
    return;
  }
  //------------------------------------------------------
  switch (menuState) {

    // -------------------------------------------------
    // ГЛАВНОЕ МЕНЮ
    // -------------------------------------------------
    case STATE_MAIN_MENU:
      if (key == "1") setMenuState(STATE_NEWGAME_NUMPLAYERS);
      if (key == "2" && gameActive) setMenuState(STATE_GAME_WAITCARD);
      if (key == "3") setMenuState(STATE_SETTINGS_MENU);
      if (key == "4") setMenuState(MENU_HELP);
      break;

      // -----------------------------
      // МЕНЮ НАСТРОЕК
      // -----------------------------
    case STATE_SETTINGS_MENU:
      if (key == "E") {
        setMenuState(STATE_MAIN_MENU);
        break;
      }
      if (key == "1") {
        setMenuState(STATE_SETTINGS_GAME);
        break;
      }
      if (key == "2") {
        setMenuState(STATE_SETTINGS_SYSTEM);
        break;
      }
      if (key == "3") {
        setMenuState(STATE_SETTINGS_SERVICE);
        break;
      }
      if (key == "#") {
        setMenuState(STATE_SETTINGS_SAVE);
        break;
      }
      break;

    case STATE_SETTINGS_SYSTEM:
      if (key == "1") {
        setMenuState(STATE_SETTINGS_CURRENCY);
        break;
      }
      if (key == "2") {
        setMenuState(STATE_SETTINGS_LANGUAGE);
        break;
      }
      if (key == "3") { break; }  // пока пусто
      if (key == "4") {
        setMenuState(STATE_SETTINGS_SYSINFO);
        break;
      }
      if (key == "E") {
        setMenuState(STATE_SETTINGS_MENU);
        break;
      }
      break;

    case STATE_SETTINGS_CURRENCY:
      if (key == "1") {
        settings.currency = 0;
        setMenuState(STATE_SETTINGS_CURRENCY);
        break;
      }
      if (key == "2") {
        settings.currency = 1;
        setMenuState(STATE_SETTINGS_CURRENCY);
        break;
      }
      if (key == "3") {
        settings.currency = 2;
        setMenuState(STATE_SETTINGS_CURRENCY);
        break;
      }
      if (key == "4") {
        settings.currency = 3;
        setMenuState(STATE_SETTINGS_CURRENCY);
        break;
      }
      if (key == "N") {
        setMenuState(STATE_SETTINGS_SYSTEM);
        break;
      }
      break;

    case STATE_SETTINGS_LANGUAGE:
      if (key == "N") {
        setMenuState(STATE_SETTINGS_SYSTEM);
        break;
      }
      if (key == "1") {
        settings.language = 0;
        uiShowSettingsLanguage();  // обновить отображение
        break;
      }
      if (key == "2") {
        settings.language = 1;
        uiShowSettingsLanguage();  // обновить отображение
        break;
      }
      break;

    case STATE_SETTINGS_SYSINFO:
      if (key == "E") { setMenuState(STATE_SETTINGS_SYSTEM); }
      break;

    case STATE_SETTINGS_RTC_TEST:
    case STATE_SETTINGS_BATTERY_TEST:
      if (key == "E") { setMenuState(STATE_SETTINGS_SERVICE); }
      break;

    // -----------------------------
    // ПОДМЕНЮ НАСТРОЕК ИГРЫ
    // -----------------------------
    case STATE_SETTINGS_GAME:
      if (key == "E") {
        setMenuState(STATE_SETTINGS_MENU);
        break;
      }
      if (key == "1") {
        tempValue = settings.maxBalance;
        setMenuState(STATE_SETTINGS_GAME_EDIT_MAXBALANCE);
        break;
      }
      if (key == "2") {
        settings.confirmLargeOps = !settings.confirmLargeOps;
        uiShowSettingsGame();
        break;
      }
      if (key == "3") {
        tempValue = settings.largeOpThreshold;
        setMenuState(STATE_SETTINGS_GAME_EDIT_THRESHOLD);
        break;
      }
      if (key == "4") {
        settings.autoEndGame = !settings.autoEndGame;
        uiShowSettingsGame();
        break;
      }
      break;

    case STATE_SETTINGS_GAME_EDIT_MAXBALANCE:
      if (key == "E") {
        setMenuState(STATE_SETTINGS_GAME);
        break;
      }
      if (key == "N") {
        settings.maxBalance = tempValue;
        setMenuState(STATE_SETTINGS_GAME);
        break;
      }
      if (key >= "0" && key <= "9") {
        tempValue = tempValue * 10 + key.toInt();
        uiShowEditMaxBalance(tempValue);
        break;
      }
      if (key == "L") {
        tempValue /= 10;  // удаление последней цифры
        uiShowEditMaxBalance(tempValue);
        break;
      }
      break;

    case STATE_SETTINGS_GAME_EDIT_THRESHOLD:
      if (key == "E") {
        setMenuState(STATE_SETTINGS_GAME);
        break;
      }
      if (key == "N") {
        settings.largeOpThreshold = tempValue;
        setMenuState(STATE_SETTINGS_GAME);
        break;
      }
      if (key >= "0" && key <= "9") {
        tempValue = tempValue * 10 + key.toInt();
        uiShowEditThreshold(tempValue);
        break;
      }
      if (key == "L") {
        tempValue /= 10;
        uiShowEditThreshold(tempValue);
        break;
      }
      break;

    case STATE_SETTINGS_SERVICE:
      if (key == "E") {
        setMenuState(STATE_SETTINGS_MENU);
        break;
      }
      if (key == "1") {
        setMenuState(STATE_SETTINGS_RTC_TEST);
        break;
      }
      if (key == "2") {
        setMenuState(STATE_SETTINGS_BATTERY_TEST);
        break;
      }
      if (key == "3") {
        drawScreen("WiFi", "...", "...", "...", "[Esc] Назад");
        break;
      }
      break;

    // -------------------------------------------------
    // МАСТЕР НОВОЙ ИГРЫ
    // -------------------------------------------------
    case STATE_NEWGAME_NUMPLAYERS:
      if ((key == "U" || key == "R") && selectedPlayerCount < 8) {
        selectedPlayerCount++;
        uiShowNewGame_NumPlayers(selectedPlayerCount);
      }
      if ((key == "D" || key == "L") && selectedPlayerCount > 2) {
        selectedPlayerCount--;
        uiShowNewGame_NumPlayers(selectedPlayerCount);
      }
      if (key == "N") {
        for (int i = 0; i < 8; i++) {
          players[i].name = nullptr;
          memset(players[i].uid, 0, 4);
          players[i].balance = 0;
        }
        currentRegisteringPlayer = 0;
        setMenuState(STATE_NEWGAME_REGCARD);
      }
      if (key == "E") setMenuState(STATE_MAIN_MENU);
      break;

    case STATE_NEWGAME_REGCARD:
      if (key == "N") {
        if (currentRegisteringPlayer < selectedPlayerCount)
          setMenuState(STATE_NEWGAME_REGCARD);
        else
          setMenuState(STATE_NEWGAME_STARTBALANCE);
      }
      if (key == "E") {
        currentRegisteringPlayer = 0;
        setMenuState(STATE_NEWGAME_NUMPLAYERS);
      }
      break;

    case STATE_NEWGAME_STARTBALANCE:
      if ((key == "U" || key == "R") && selectedStartBalance < 3000) {
        selectedStartBalance += 500;
        uiShowNewGame_StartBalance(selectedStartBalance);
      }
      if ((key == "D" || key == "L") && selectedStartBalance > 500) {
        selectedStartBalance -= 500;
        uiShowNewGame_StartBalance(selectedStartBalance);
      }
      if (key == "N") setMenuState(STATE_NEWGAME_CONFIRM);
      if (key == "E") setMenuState(STATE_NEWGAME_REGCARD);
      break;

    case STATE_NEWGAME_CONFIRM:
      if (key == "*") {
        startBalance = selectedStartBalance;
        resetAllBalances();
        gameStartTime = rtc.now();
        gameActive = true;
        saveGameState();
        setMenuState(STATE_GAME_WAITCARD);
      }
      if (key == "#") setMenuState(STATE_NEWGAME_STARTBALANCE);
      break;

    // -------------------------------------------------
    // ИГРОВОЙ РЕЖИМ
    // -------------------------------------------------
    case STATE_GAME_WAITCARD:

      if (key == "#") {
        saveGameState();
        setMenuState(STATE_MAIN_MENU);
      }
      break;

    case STATE_GAME_PLAYERMENU:
      if (key == "1") {
        // Перевод игроку: сначала выбираем игрока, потом сумму
        setMenuState(STATE_GAME_TRANSFER_SELECTPLAYER);
      }

      if (key == "2") {  // К ОПЛАТЕ БАНКУ
        transferAmount = 0;
        amountNextState = STATE_GAME_BANK_PAY;  // после выбора суммы пойдём сюда
        setMenuState(STATE_GAME_SUM_PRESET);
      }

      if (key == "3") {  // ПОЛУЧИТЬ ОТ БАНКА
        transferAmount = 0;
        amountNextState = STATE_GAME_BANK_GET;  // после выбора суммы пойдём сюда
        setMenuState(STATE_GAME_SUM_PRESET);
      }
      if (key == "4") {
        setMenuState(STATE_GAME_PLAYER_FINISH);
      }


      if (key == "E") setMenuState(STATE_GAME_WAITCARD);
      break;

    case STATE_GAME_TRANSFER_SELECTPLAYER:
      if (key == "E") setMenuState(STATE_GAME_PLAYERMENU);
      if (key == "*") {
        transferTargetIndex = -1;  // специальный код: всем игрокам
        transferAmount = 0;
        amountNextState = STATE_GAME_TRANSFER_SUM;
        setMenuState(STATE_GAME_SUM_PRESET);
        break;
      }

      if (key.length() == 1 && key[0] >= '1' && key[0] <= '8') {
        int idx = key.toInt() - 1;
        if (idx != activePlayerIndex && idx < selectedPlayerCount) {
          transferTargetIndex = idx;
          transferAmount = 0;
          amountNextState = STATE_GAME_TRANSFER_SUM;
          setMenuState(STATE_GAME_SUM_PRESET);
        }
      }
      break;

    case STATE_GAME_PLAYER_FINISH:
      if (key == "E") {
        setMenuState(STATE_GAME_PLAYERMENU);
        break;
      }

      if (key == "N") {
        players[activePlayerIndex].balance = 0;
        players[activePlayerIndex].eliminated = true;
        // lastAction.valid = false;
        saveGameState();
        if (!checkAutoEndGame()) {
          setMenuState(STATE_GAME_WAITCARD);
        }
        break;
      }
      char buf[16];
      snprintf(buf, sizeof(buf), "Игрок %d", activePlayerIndex + 1);
      uiShowPlayerFinish(buf);
      break;

      // -------------------------------------------------
      // ВЫБОР ФИКСИРОВАННЫХ СУММ
      // -------------------------------------------------
    case STATE_GAME_SUM_PRESET:

      if (key == "E") {
        setMenuState(STATE_GAME_PLAYERMENU);
        break;
      }

      // Цифры 1–5 → выбор preset
      if (key.length() == 1 && key[0] >= '1' && key[0] <= '5') {
        int idx = key.toInt() - 1;
        if (idx >= 0 && idx < 5) {
          transferAmount = presetAmounts[idx];
          setMenuState(amountNextState);
          break;
        }
      }

      // Ручной ввод
      if (key == "*") {
        transferAmount = 0;
        setMenuState(STATE_GAME_SUM_MANUAL);
        uiShowGame_SumManual(transferAmount);
        break;
      }
      break;

      // -------------------------------------------------
      // РУЧНОЙ ВВОД СУММЫ
      // -------------------------------------------------
    case STATE_GAME_SUM_MANUAL:
      if (key == "E") {
        setMenuState(STATE_GAME_SUM_PRESET);
        break;
      }
      // Удаление последней цифры
      if (key == "L") {
        transferAmount /= 10;
        uiShowGame_SumManual(transferAmount);
        break;
      }
      // Ввод цифр
      if (key.length() == 1 && key[0] >= '0' && key[0] <= '9') {
        // Ограничение длины: максимум 5 цифр
        if (transferAmount <= 99999) {
          long digit = key.toInt();
          long newValue = transferAmount * 10 + digit;
          // Не даём выйти за пределы 5 цифр
          if (newValue <= 99999) {
            transferAmount = newValue;
            uiShowGame_SumManual(transferAmount);
          }
        }
        break;
      }
      // Подтверждение суммы
      if (key == "N" && transferAmount > 0) {
        // 1. Жёсткий лимит операции
        if (transferAmount > maxOperationAmount) {
          uiShowGame_Result("Ошибка!", "Лимит операции", "");
          setMenuState(STATE_GAME_RESULT);
          break;
        }
        // 2. Порог подтверждения крупной операции
        if (settings.confirmLargeOps && transferAmount >= settings.largeOpThreshold) {
          setMenuState(STATE_GAME_CONFIRM_LARGE);
          uiShowConfirmLarge(transferAmount);
          break;
        }
        // 3. Проверка лимита баланса (только для получения)
        if (amountNextState == STATE_GAME_BANK_GET) {
          long allowed = settings.maxBalance - players[activePlayerIndex].balance;
          if (transferAmount > allowed) {
            uiShowGame_Result("Ошибка!", "Лимит баланса", "");
            setMenuState(STATE_GAME_RESULT);
            break;
          }
        }
        // 4. Всё ок — переходим к операции
        setMenuState(amountNextState);
      }
      break;

    // -------------------------------------------------
    // ПОДТВЕРЖДЕНИЕ ОПЕРАЦИИ
    // -------------------------------------------------
    case STATE_GAME_TRANSFER_SUM:
    case STATE_GAME_BANK_PAY:
    case STATE_GAME_BANK_GET:

      if (key == "E") {
        setMenuState(STATE_GAME_PLAYERMENU);
        break;
      }

      if (key == "N" && transferAmount > 0) {
        long realAmount = 0;
        char l2[32] = { 0 }, l3[32] = { 0 }, l4[32] = { 0 };

        switch (menuState) {
          case STATE_GAME_TRANSFER_SUM:
            // перевод всем игрокам
            if (transferTargetIndex == -1) {
              TransferAllResult res = doTransferAll(activePlayerIndex, transferAmount);
              if (!gameActive) {
                transferAmount = 0;
                break;
              }
              if (res.totalGiven > 0) {
                long realPerPlayer = res.totalGiven / res.activeCount;
                snprintf(l2, sizeof(l2), "Игрок %d:", activePlayerIndex + 1);
                snprintf(l3, sizeof(l3), "По %ld%s каждому", realPerPlayer, names[settings.currency]);
                snprintf(l4, sizeof(l4), "Всего: %ld%s", res.totalGiven, names[settings.currency]);
                realAmount = res.totalGiven;
              }
              break;
            }
            // перевод конкретному игроку
            realAmount = doTransfer(activePlayerIndex, transferTargetIndex, transferAmount);
            if (!gameActive) {
              transferAmount = 0;
              break;
            }
            if (realAmount > 0) {
              snprintf(l2, sizeof(l2), "Игрок %d >> %d",
                       activePlayerIndex + 1,
                       transferTargetIndex + 1);
              snprintf(l3, sizeof(l3), "Сумма: %ld%s", realAmount, names[settings.currency]);
            }
            break;


          case STATE_GAME_BANK_PAY:
            realAmount = doPayBank(activePlayerIndex, transferAmount);
            if (!gameActive) {
              transferAmount = 0;
              break;
            }
            if (realAmount > 0) {
              snprintf(l2, sizeof(l2), "Оплата банку");
              snprintf(l3, sizeof(l3), "Сумма: %ld%s", realAmount, names[settings.currency]);
            }
            break;

          case STATE_GAME_BANK_GET:
            realAmount = doGetBank(activePlayerIndex, transferAmount);
            if (!gameActive) {
              transferAmount = 0;
              break;
            }
            if (realAmount > 0) {
              snprintf(l2, sizeof(l2), "Получено");
              snprintf(l3, sizeof(l3), "Сумма: %ld%s", realAmount, names[settings.currency]);
            }
            break;
        }

        // Универсальная обработка ошибок
        if (realAmount == 0) {
          uiShowGame_Result("Ошибка!", "Операция невозможна", "");
          transferAmount = 0;
          setMenuState(STATE_GAME_RESULT);
          break;
        }
        if (!gameActive) break;

        // Успех
        uiShowGame_Result(l2, l3, l4);
        transferAmount = 0;
        saveGameState();
        setMenuState(STATE_GAME_RESULT);
      }

      break;

    case STATE_GAME_FINISH:
      if (key == "N" || key == "E" || key == "#") {
        setMenuState(STATE_MAIN_MENU);
      }
      break;

    case STATE_GAME_RESULT:
      if (key == "N") setMenuState(STATE_GAME_PLAYERMENU);
      break;

    case STATE_GAME_BALANCES:
      if (key == "E" || key == "H") {
        setMenuState(returnFromBalances);
      }
      break;

    case STATE_GAME_CONFIRM_LARGE:
      if (key == "E") {
        setMenuState(STATE_GAME_SUM_MANUAL);
        uiShowGame_SumManual(transferAmount);
        break;
      }
      if (key == "N") {
        setMenuState(amountNextState);
        break;
      }
      break;
  }
}
