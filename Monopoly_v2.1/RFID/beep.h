#pragma once

#include <Arduino.h>

#define BUZZER_PIN A2

extern const int Md;
extern const int Ld;
extern const int Sd;

void beepTone(int freq, int dur);
void beepDigit(char k);
void beepArrow();
void beepEnter();
void beepEsc();
void beepDelete();
void beepService(char k);
void beepCard();