#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN A2
#define PIN_BACKLIGHT 6

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Клавиатура 4x5
const byte ROWS = 5;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { 'L', '0', 'R', 'N' },
  { '7', '8', '9', 'E' },
  { '4', '5', '6', 'D' },
  { 'F', 'H', '#', '*' },
  { '1', '2', '3', 'U' }
};

byte rowPins[ROWS] = { A3, 7, 8, 14, 15 };
byte colPins[COLS] = { 2, 3, 4, 5 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String lastUID = "";       // последний считанный UID
bool cardPresent = false;  // флаг "карта сейчас поднесена"

void beepTone(int freq, int dur) {
  tone(BUZZER_PIN, freq, dur);
}

void beepDigit(char k) {
  int base = 900;
  if (k == '0') beepTone(1400, 40);
  else beepTone(base + (k - '1') * 50, 40);
}

void beepArrow() {
  beepTone(3250, 20);
}

void beepEnter() {
  beepTone(1800, 40);
}

void beepEsc() {
  beepTone(600, 40);
}

void beepDelete() {
  beepTone(500, 60);
}

void beepService(char k) {
  if (k == '#') beepTone(1600, 40);
  else if (k == '*') beepTone(800, 40);
  else if (k == 'F' || k == 'H') beepTone(2000, 25);
}

void beepCard() {
  beepTone(1200, 40);
  delay(80);
  beepTone(1500, 40);
  delay(80);
  beepTone(1800, 80);
}
// подсветка
bool backlightOn = true;
byte levels[] = { 0, 20, 50, 160, 255 };
int levelIndex = 4;  // стартуем с 100%
unsigned long lastKeyTime = 0;
const unsigned long AUTO_OFF_MS = 2UL * 60UL * 1000UL;  // 5 минут
bool autoDimmed = false;
byte currentPWM = 0;

void fadeTo(byte target) {
  if (target == currentPWM) return;
  int step = (target > currentPWM) ? 1 : -1;
  for (int v = currentPWM; v != target; v += step) {
    analogWrite(PIN_BACKLIGHT, v);
    delay(3);  // скорость плавности
  }
  currentPWM = target;
}

void onF1Pressed() {
  levelIndex++;
  if (levelIndex > 4) levelIndex = 0;
  // analogWrite(PIN_BACKLIGHT, levels[levelIndex]);
  byte target = levels[levelIndex];
  fadeTo(target);
  EEPROM.update(0, levelIndex);
}


void setup() {
  Serial.begin(115200);
  // EEPROM.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  // читаем сохранённый уровень
  levelIndex = EEPROM.read(0);
  if (levelIndex > 4) levelIndex = 4;
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, LOW);
  currentPWM = levels[levelIndex]; 
  analogWrite(PIN_BACKLIGHT, currentPWM);
  lastKeyTime = millis();

  SPI.begin();
  mfrc522.PCD_Init();
}
// ----------------------------------------------------------------------------------------------
void loop() {
  // --- RFID ---
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {

      // собираем UID в строку
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) uid += " ";
      }
      uid.toUpperCase();

      // если карта новая или другая — выводим
      if (!cardPresent || uid != lastUID) {
        beepCard();
        Serial.print("CARD ");
        Serial.println(uid);
        if (autoDimmed) {
          fadeTo(levels[levelIndex]);
          autoDimmed = false;
        }
        lastKeyTime = millis();

        lastUID = uid;
        cardPresent = true;
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  } else {
    // карты нет — сбрасываем флаг
    if (cardPresent) {
      cardPresent = false;
      lastUID = "";
    }
  }

  // автоотключение подсветки
  if (millis() - lastKeyTime > AUTO_OFF_MS) {
    fadeTo(0);
    // analogWrite(PIN_BACKLIGHT, 0);  // выключить
    autoDimmed = true;
  }

  // --- Клавиатура ---
  char key = keypad.getKey();
  if (key) {
    if (autoDimmed) {
      // analogWrite(PIN_BACKLIGHT, levels[levelIndex]);
      fadeTo(levels[levelIndex]);
      autoDimmed = false;
    }
    lastKeyTime = millis();  // сброс таймера автоотключения
    if (key >= '0' && key <= '9') beepDigit(key);
    else if (key == 'L' || key == 'R' || key == 'U' || key == 'D') beepArrow();
    else if (key == 'N') beepEnter();
    else if (key == 'E') beepEsc();
    else if (key == 'D') beepDelete();
    else if (key == 'F') {
      onF1Pressed();
      beepService(key);
    } else beepService(key);

    Serial.print("KEY ");
    Serial.println(key);
  }
}
