#include <button.h>
#include <Arduino.h>

#include <scheduler.h>
#include <RTC.h>
#include <savefile.h>
#include <wifi.h>
#include <statusLED.h>


#define BUTTON_PIN 12


volatile bool buttonPressed = false;
volatile bool buttonReleased = false;

volatile unsigned long pressStart = 0;
volatile unsigned long pressTime = 0;


void IRAM_ATTR buttonISR()
{
    bool state = digitalRead(BUTTON_PIN);

    if(state == LOW)
    {
        if(!buttonPressed)
        {
            buttonPressed = true;
            pressStart = millis();
        }
    }
    else
    {
        if(buttonPressed)
        {
            pressTime = millis() - pressStart;
            buttonPressed = false;
            buttonReleased = true;
        }
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
    if(!buttonReleased)
        return;


    noInterrupts();

    unsigned long duration = pressTime;
    buttonReleased = false;

    interrupts();



    Serial.print("Button duration: ");
    Serial.println(duration);



    if(duration < 500)
    {
        Serial.println("Noise");
        return;
    }


    if(duration >= 20000)
    {
        Serial.println("20sec Press");

        ResetSchedules();
        removeFile(SCHEDULE_FILE);

        systemtimestamp = 0;
        SysTS_RTC();

        setLEDInterval(100);
        resetLEDInterval(10.f);

        return;
    }



    if(duration >= 10000)
    {
        Serial.println("10sec Press");

        removeFile(WIFICONFIG_FILE);

        setLEDInterval(300);
        resetLEDInterval(10.f);

        return;
    }



    if(duration >= 3000)
    {
        Serial.println("3sec Press");

        wifi_toggle_onoff();

        setLEDInterval(50);
        resetLEDInterval(2.f);

        return;
    }


    Serial.println("Short Press");
}