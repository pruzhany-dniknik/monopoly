#pragma once
#include <Arduino.h>

void inputUpdate();
void processLine(const char* line) ;
bool parseUID_C(const char* s, byte out[4]);
void handleCard(byte uid[4]) ;

int batteryPercent(float v);
float readBatteryVoltage();

