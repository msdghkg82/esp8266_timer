#include "DateConverter.h"
#include <time.h>



Date_t gregorianToPersian_ChatGPT(int gy, int gm, int gd)
{
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

bool isPersianLeapYear_ChatGPT(int jy)
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

/**************************************
 * 
 *   CLAUDE
 * 
***************************************/

/*
 * gregorian_to_persian.c
 * ------------------------------------------------------------------
 * Gregorian -> Persian (Jalali/Shamsi) calendar conversion.
 *
 * DESIGN GOAL: gregorianToPersian() makes NO assumption about which
 * years are leap. It treats isPersianLeapYear(int) as the single
 * source of truth and asks it a question ("is year Y leap?") instead
 * of baking in a cycle formula (33-year, 2820-year, etc.). Swap the
 * body of isPersianLeapYear() for a different algorithm and this file
 * does not need to change.
 *
 * Compiles as C (C99+) or C++.
 * ------------------------------------------------------------------
 */

#include <stdbool.h>

/* ------------------------------------------------------------------
 * isPersianLeapYear — REPLACEABLE.
 *
 * This is one correct implementation (the 33-year / grand-cycle rule
 * used by jalaali-js, Borkowski's paper, etc.) provided so the file
 * compiles and works out of the box. gregorianToJDN() and
 * gregorianToPersian() below never look at its internals — they only
 * ever call it — so you can replace this function's body with any
 * other leap-year rule and the rest of the file keeps working.
 * ------------------------------------------------------------------ */
bool isPersianLeapYear_Claude(int jy)
{
    static const int breaks[] = {
        -61, 9, 38, 199, 426, 686, 756, 818, 1111, 1181, 1210,
        1635, 2060, 2097, 2192, 2262, 2324, 2394, 2456, 3178
    };
    const int bl = sizeof(breaks) / sizeof(breaks[0]);

    int jp = breaks[0];
    int leapJ = -14;
    int jump = 0;
    int i, jm, n;

    for (i = 1; i < bl; i++) {
        jm = breaks[i];
        jump = jm - jp;
        if (jy < jm)
            break;
        leapJ = leapJ + (jump / 33) * 8 + (jump % 33) / 4;
        jp = jm;
    }
    n = jy - jp;

    leapJ = leapJ + (n / 33) * 8 + ((n % 33) + 3) / 4;
    if ((jump % 33) == 4 && (jump - n) == 4)
        leapJ++;

    if (jump - n < 6)
        n = n - jump + ((jump + 4) / 33) * 33;

    int leap = ((n + 1) % 33 - 1) % 4;
    if (leap == -1)
        leap = 4;

    return leap == 0;
}

/* ------------------------------------------------------------------
 * gregorianToJDN — Gregorian calendar date -> Julian Day Number.
 *
 * This is plain calendrical arithmetic (Fliegel & Van Flandern),
 * unrelated to the Persian calendar or its leap rule, so it is safe
 * to keep as a closed-form formula. Verified against known JDN
 * reference points (e.g. JDN(2000-01-01) = 2451545).
 * ------------------------------------------------------------------ */
long gregorianToJDN(int gy, int gm, int gd)
{
    long a = (14 - gm) / 12;
    long y = gy + 4800 - a;
    long m = gm + 12 * a - 3;
    return gd + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
}

/* Julian Day Number of 1 Farvardin, Persian year 1. This is the
 * calendar's fixed starting point (day 0), true by definition
 * regardless of which years turn out to be leap afterward — so,
 * unlike a cumulative-day-count shortcut, it does NOT encode any
 * assumption about the leap pattern. */
#define PERSIAN_EPOCH 1948321L

/* ------------------------------------------------------------------
 * gregorianToPersian — the function you asked for.
 *
 * Algorithm:
 *   1. Convert the Gregorian date to an absolute day count (JDN).
 *   2. Measure days elapsed since the Persian epoch.
 *   3. Walk forward year by year, subtracting 365 or 366 days
 *      (whichever isPersianLeapYear(jy) says) until the remaining
 *      day count fits inside the current year. This is the only
 *      part of the algorithm that "knows" about leap years, and it
 *      never assumes their positions — it just asks, one year at a
 *      time.
 *   4. Within that year, walk month by month using the fixed Persian
 *      month-length table, where month 12 (Esfand) is 29 or 30 days
 *      — again decided by calling isPersianLeapYear(jy), never
 *      hard-coded.
 *
 * Cost: O(number of years since the epoch) per call — about 1400
 * iterations of a trivial integer loop for a present-day date. On an
 * ESP8266 that is on the order of a few microseconds, which is
 * irrelevant unless this runs inside a tight loop (it shouldn't —
 * call it once per date you need, e.g. at NTP sync).
 * ------------------------------------------------------------------ */
Date_t gregorianToPersian_Claude(int gy, int gm, int gd)
{
    long jdn = gregorianToJDN(gy, gm, gd);
    long remaining = jdn - PERSIAN_EPOCH;   /* days since 1 Farvardin, year 1 */

    int jy = 1;
    while (1) {
        long yearLen = isPersianLeapYear_Claude(jy) ? 366 : 365;
        if (remaining < yearLen)
            break;
        remaining -= yearLen;
        jy++;
    }

    const int monthLengths[12] = {
        31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30,
        isPersianLeapYear_Claude(jy) ? 30 : 29
    };

    int jm = 1;
    for (int i = 0; i < 12; i++) {
        if (remaining < monthLengths[i]) {
            jm = i + 1;
            break;
        }
        remaining -= monthLengths[i];
    }

    Date_t result;
    result.year = jy;
    result.month = jm;
    result.day = (int)remaining + 1;
    return result;
}
