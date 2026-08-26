#ifndef DATECONVERTER_H
#define DATECONVERTER_H



typedef struct {
    int year;
    int month;
    int day;
} Date_t;

Date_t gregorianToPersian(const struct tm& date);
bool isPersianLeapYear_(int jy);


#endif // DATECONVERTER_H