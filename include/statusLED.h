#ifndef STATUSLED_H
#define STATUSLED_H

#pragma once
#include <stdint.h>

void statusLED_Init();
void setLEDInterval(uint32_t interval);
void resetLEDInterval();

#endif // STATUSLED_H