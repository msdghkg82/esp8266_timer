#ifndef DATECONVERTER_H
#define DATECONVERTER_H



struct Date_
{
    int year;
    int month;
    int day;
};

Date_ gregorianToPersian(const struct tm& date);
bool isPersianLeapYear_(int jy);


#endif // DATECONVERTER_H