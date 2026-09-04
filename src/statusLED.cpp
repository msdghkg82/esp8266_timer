#include <statusLED.h>
#include <Arduino.h>
#include <Ticker.h>

Ticker ledTicker;
Ticker ledTicker_reset;


void ledTicker_callback()
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle the LED state
}

void ledTicker_reset_callback()
{
    setLEDInterval(1000);
}

void statusLED_Init()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Turn off the LED (assuming active low)
    ledTicker.attach_ms(1000, ledTicker_callback); // Start the ticker with a default interval of 1 second
}

void setLEDInterval(uint32_t interval)
{
    ledTicker.detach(); // Stop the current ticker
    ledTicker.attach_ms(interval, ledTicker_callback); // Start a new ticker with the updated interval
}

void resetLEDInterval(float_t resetTime)
{
    ledTicker_reset.once(resetTime, ledTicker_reset_callback);
}