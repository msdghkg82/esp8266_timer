#ifndef DATECONVERTER_H
#define DATECONVERTER_H

#pragma once
#include <time.h>

typedef struct{
    int year;
    int month;
    int day;
} Date_t;

Date_t gregorianToPersian_ChatGPT(int gy, int gm, int gd);
bool isPersianLeapYear_ChatGPT(int jy);

Date_t gregorianToPersian_Claude(int gy, int gm, int gd);
bool isPersianLeapYear_Claude(int jy);
time_t MyTimegm(const struct tm *tm);


#endif // DATECONVERTER_H