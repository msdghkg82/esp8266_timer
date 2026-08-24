#include "DateConverter.h"
#include <time.h>



Date_ gregorianToPersian(const struct tm& date)
{
    int gy = date.tm_year + 1900;
    int gm = date.tm_mon + 1;
    int gd = date.tm_mday;

    int g_d_m[12] =
    {
        0, 31, 59, 90, 120, 151,
        181, 212, 243, 273, 304, 334
    };

    int gy2 = (gm > 2) ? (gy + 1) : gy;

    long days =
        355666L
        + (365L * gy)
        + ((gy2 + 3) / 4)
        - ((gy2 + 99) / 100)
        + ((gy2 + 399) / 400)
        + gd
        + g_d_m[gm - 1];

    int jy = -1595 + (33 * (days / 12053));
    days %= 12053;

    jy += 4 * (days / 1461);
    days %= 1461;

    if (days > 365)
    {
        jy += (days - 1) / 365;
        days = (days - 1) % 365;
    }

    int jm;

    if (days < 186)
        jm = 1 + (days / 31);
    else
        jm = 7 + ((days - 186) / 30);

    int jd;

    if (days < 186)
        jd = 1 + (days % 31);
    else
        jd = 1 + ((days - 186) % 30);

    return {jy, jm, jd};
}

bool isPersianLeapYear_(int jy)
{
    switch (jy)
    {
        case 1391:
        case 1395:
        case 1399:
        case 1403:
        case 1408:
        case 1412:
        case 1416:
        case 1420:
        case 1424:
        case 1428:
        case 1432:
        case 1436:
            return true;

        default:
            return false;
    }
}