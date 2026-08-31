#include <button.h>
#include <Arduino.h>

#include <scheduler.h>
#include <RTC.h>
#include <savefile.h>
#include <wifi.h>
#include <statusLED.h>

#define BUTTON_PIN 12

volatile uint32_t pressStartTime = 0;
volatile uint32_t pressDuration = 0;
volatile bool buttonReleased = false;


void IRAM_ATTR buttonISR()
{
    if (digitalRead(BUTTON_PIN) == LOW)
    {
        // Falling edge
        pressStartTime = millis();
    }
    else
    {
        // Rising edge
        pressDuration = millis() - pressStartTime;
        buttonReleased = true;
    }
}


void ButtonInit()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        CHANGE
    );
}


void ButtonTask()
{
    if(buttonReleased)
    {
        buttonReleased = false;

        if(pressDuration < 500)
        {
            Serial.println("Noise");
            return; // Ignore short presses
        }
        else if(pressDuration >= 20000)
        {
            Serial.println("20 second press");
            ResetSchedules();
            removeFile(SCHEDULE_FILE);
            systemtimestamp = 0;
            setLEDInterval(100); // Set LED blink interval to 100 ms
            resetLEDInterval();
        }
        else if(pressDuration >= 10000)
        {
            Serial.println("10 second press");
            removeFile(WIFICONFIG_FILE);
            setLEDInterval(300); // Set LED blink interval to 300 ms
            resetLEDInterval();
        }
        else
        {
            Serial.println("Short press");
        }
    }
}