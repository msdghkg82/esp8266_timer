#ifndef LOG_H
#define LOG_H

#pragma once
#include <WString.h>

#define LOG_FILE "/log.txt"

typedef enum {
    english = 0,
    persian 
} LogLang_t;

extern LogLang_t LogLang;

void Log(const String& message);

#endif // LOG_H