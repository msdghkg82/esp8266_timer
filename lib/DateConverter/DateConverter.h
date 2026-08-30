#ifndef DATECONVERTER_H
#define DATECONVERTER_H



typedef struct Date_t{
    int year;
    int month;
    int day;
};

Date_t gregorianToPersian_ChatGPT(int gy, int gm, int gd);
bool isPersianLeapYear_ChatGPT(int jy);

Date_t gregorianToPersian_Claude(int gy, int gm, int gd);
bool isPersianLeapYear_Claude(int jy);


#endif // DATECONVERTER_H