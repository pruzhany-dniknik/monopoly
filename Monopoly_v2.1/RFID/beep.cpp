#include "beep.h"

const int Md = 6;
const int Ld = 15;
const int Sd = 20;

void beepTone(int freq, int dur) {
  tone(BUZZER_PIN, freq, dur);
}

void beepDigit(char k) {
  int base = 900;
  if (k == '0') beepTone(1400, Md);
  else beepTone(base + (k - '1') * 50, Md);
}

void beepArrow() {
  beepTone(3250, Md);
}

void beepEnter() {
  beepTone(1800, Md);
}

void beepEsc() {
  beepTone(600, Md);
}

void beepDelete() {
  beepTone(500, Md);
}

void beepService(char k) {
  if (k == '#') beepTone(1200, Md);
  else if (k == '*') beepTone(800, Md);
  else if (k == 'F' || k == 'H') beepTone(600, Md);
}

void beepCard() {
  beepTone(1200, Md);
  delay(80);
  beepTone(1500, Md);
  delay(80);
  beepTone(1800, Ld);
}