#ifndef STATUSLED_H
#define STATUSLED_H

#pragma once
#include <math.h>

void statusLED_Init();
void setLEDInterval(uint32_t interval);
void resetLEDInterval(float_t resetTime);

#endif // STATUSLED_H