#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <avr/wdt.h>
#include "beep.h"

#define SS_PIN 10
#define RST_PIN 9
#define PIN_BACKLIGHT 6

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Клавиатура 4x5
const byte ROWS = 5, COLS = 4;
char keys[ROWS][COLS] = {
  { 'L', '0', 'R', 'N' },
  { '7', '8', '9', 'E' },
  { '4', '5', '6', 'D' },
  { '1', '2', '3', 'U' },
  { 'F', 'H', '#', '*' }
};

byte rowPins[ROWS] = { 7, 8, 14, 15, 17 };
byte colPins[COLS] = { 2, 3, 4, 5 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String lastUID = "";
bool cardPresent = false;

// подсветка
byte levels[] = { 0, 20, 50, 160, 255 };
int levelIndex = 2;
unsigned long lastKeyTime = 0;
const unsigned long AUTO_OFF_MS = 2UL * 60UL * 1000UL;
bool autoDimmed = false;
byte currentPWM = 0;

void fadeTo(byte target) {
  if (target == currentPWM) return;
  int step = (target > currentPWM) ? 1 : -1;
  for (int v = currentPWM; v != target; v += step) {
    analogWrite(PIN_BACKLIGHT, v);
    delay(3);
  }
  currentPWM = target;
}

void onF1Pressed() {
  levelIndex++;
  if (levelIndex > 4) levelIndex = 0;
  fadeTo(levels[levelIndex]);
}

// ========== УПРОЩЁННЫЙ СБРОС СКАНЕРА ==========
void forceResetRFID() {
  Serial.println("Force reset RFID...");
  
  // 1. Аппаратный сброс через пин RST
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(200);
  
  // 2. Перезапуск SPI
  SPI.end();
  delay(100);
  
  // 3. Включаем обратно
  digitalWrite(RST_PIN, HIGH);
  delay(200);
  
  // 4. Заново инициализируем
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  mfrc522.PCD_Init();
  delay(100);
  mfrc522.PCD_AntennaOn();
  
  // 5. Сбрасываем флаги
  cardPresent = false;
  lastUID = "";
  
  Serial.println("RFID reset done");
}

// ========== ПРОВЕРКА РАБОТОСПОСОБНОСТИ ==========
bool isRFIDAlive() {
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  return (version == 0x92 || version == 0x91 || version == 0x93);
}

// ========== ЧТЕНИЕ КАРТЫ С ТАЙМАУТОМ ==========
bool readCard() {
  unsigned long start = millis();
  
  // Ждём появления карты (макс 300 мс)
  while (!mfrc522.PICC_IsNewCardPresent()) {
    if (millis() - start > 300) return false;
    delay(1);
    wdt_reset();
  }
  
  // Читаем карту (макс 300 мс)
  start = millis();
  while (!mfrc522.PICC_ReadCardSerial()) {
    if (millis() - start > 300) return false;
    delay(1);
    wdt_reset();
  }
  return true;
}

// ========== SETUP ==========
void setup() {
  Serial.begin(19200);
  wdt_enable(WDTO_2S);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  pinMode(PIN_BACKLIGHT, OUTPUT);
  currentPWM = levels[levelIndex];
  analogWrite(PIN_BACKLIGHT, currentPWM);
  lastKeyTime = millis();
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  mfrc522.PCD_Init();
  
  Serial.println("RFID ready");
}

// ========== LOOP ==========
void loop() {
  wdt_reset();
  
  // --- КЛАВИАТУРА (обрабатываем всегда) ---
  char key = keypad.getKey();
  if (key) {
    if (autoDimmed) {
      fadeTo(levels[levelIndex]);
      autoDimmed = false;
    }
    lastKeyTime = millis();
    
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
  
  // --- ПОДСВЕТКА ---
  if (millis() - lastKeyTime > AUTO_OFF_MS) {
    fadeTo(0);
    autoDimmed = true;
  }
  
   // --- СКАНЕР КАРТ (быстрое чтение) ---
// Проверяем наличие карты без длительного ожидания
if (mfrc522.PICC_IsNewCardPresent()) {
  delay(10);  // Короткая задержка для стабильности
  
  if (mfrc522.PICC_ReadCardSerial()) {
    // Формируем UID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1) uid += " ";
    }
    uid.toUpperCase();
    
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
  if (cardPresent) {
    cardPresent = false;
    lastUID = "";
  }
}

// Проверка здоровья сканера (раз в 10 секунд, не мешает чтению)
static unsigned long lastHealthCheck = 0;
if (millis() - lastHealthCheck > 10000) {
  lastHealthCheck = millis();
  if (!isRFIDAlive()) {
    Serial.println("RFID dead, resetting...");
    forceResetRFID();
  }
}
}