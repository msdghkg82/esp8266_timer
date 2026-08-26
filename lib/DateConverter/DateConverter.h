#ifndef DATECONVERTER_H
#define DATECONVERTER_H



typedef struct {
    int year;
    int month;
    int day;
} Date_t;

Date_t gregorianToPersian_ChatGPT(const struct tm& date);
bool isPersianLeapYear_ChatGPT(int jy);

Date_t gregorianToPersian_Claude(const struct tm& date);
bool isPersianLeapYear_Claude(int jy);


#endif // DATECONVERTER_H