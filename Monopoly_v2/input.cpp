#include "input.h"
#include "menu.h"
#include "players.h"
#include "ui.h"

#include <SoftwareSerial.h>

SoftwareSerial nanoSerial(12, 16);

String inputLine;

void inputInit() {
  nanoSerial.begin(115200);
}

bool parseUID(String line, byte outUID[4]) {
  line.trim();
  if (!line.startsWith("CARD ")) return false;

  line = line.substring(5);

  for (int i = 0; i < 4; i++) {
    int spaceIndex = line.indexOf(' ');
    String part;

    if (spaceIndex == -1) part = line;
    else {
      part = line.substring(0, spaceIndex);
      line = line.substring(spaceIndex + 1);
    }

    outUID[i] = (byte)strtol(part.c_str(), nullptr, 16);
  }
  return true;
}

void inputUpdate() {
  while (nanoSerial.available()) {
    char c = nanoSerial.read();
    
    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {

        String line = inputLine;
        inputLine = "";

        // -----------------------------
        // ОБРАБОТКА КЛАВИШ
        // -----------------------------
        if (line.startsWith("KEY ")) {
          String k = line.substring(4);
          if (k.length() > 0) menuHandleKey(k);
        }

        // -----------------------------
        // ОБРАБОТКА КАРТЫ
        // -----------------------------
        else if (line.startsWith("CARD ")) {

          byte uid[4];
          if (parseUID(line, uid)) {

            // Мастер новой игры
            if (menuState == STATE_NEWGAME_REGCARD) {
              handleCardInNewGameWizard(uid);
              return;
            }

            // Игровой режим
            if (gameActive) {
              int idx = findPlayerByUID(uid);
              if (idx >= 0) {
                activePlayerIndex = idx;
                setMenuState(STATE_GAME_PLAYERMENU);
              }
            }
          }
        }
      }
    } else {
      inputLine += c;
      if (inputLine.length() > 64) inputLine = "";
    }
  }
}
