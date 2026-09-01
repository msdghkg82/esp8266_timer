#include <button.h>
#include <Arduino.h>

#include <scheduler.h>
#include <RTC.h>
#include <savefile.h>
#include <wifi.h>
#include <statusLED.h>
#include <log.h>

#define BUTTON_PIN 12

// -----------------------------
// Button configuration
// -----------------------------

#define BUTTON_DEBOUNCE_MS 50

// Press duration limits
#define BUTTON_NOISE_MS     500
#define BUTTON_WIFI_MS      10000
#define BUTTON_WIFI_MAX_MS  15000
#define BUTTON_RESET_MS     20000

// -----------------------------
// ISR state
// -----------------------------

volatile bool buttonEdgeDetected = false;
volatile uint32_t buttonEdgeTime = 0;
volatile bool buttonEdgeState = HIGH;

// -----------------------------
// Button state
// -----------------------------

bool buttonStableState = HIGH;
bool buttonLastRawState = HIGH;

uint32_t buttonLastChangeTime = 0;
uint32_t pressStartTime = 0;


// ============================================================
// ISR
// ============================================================

void IRAM_ATTR buttonISR()
{
    buttonEdgeState = digitalRead(BUTTON_PIN);
    buttonEdgeTime = millis();
    buttonEdgeDetected = true;
}


// ============================================================
// Initialization
// ============================================================

void ButtonInit()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    buttonStableState = digitalRead(BUTTON_PIN);
    buttonLastRawState = buttonStableState;

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonISR,
        CHANGE
    );
}


// ============================================================
// Button task
// ============================================================

void ButtonTask()
{
    bool rawState;
    uint32_t edgeTime;

    // --------------------------------------------------------
    // Get latest state reported by ISR
    // --------------------------------------------------------

    if (buttonEdgeDetected)
    {
        noInterrupts();

        rawState = buttonEdgeState;
        edgeTime = buttonEdgeTime;

        buttonEdgeDetected = false;

        interrupts();

        // Ignore duplicate edge reports
        if (rawState != buttonLastRawState)
        {
            buttonLastRawState = rawState;
            buttonLastChangeTime = edgeTime;
        }
    }

    // --------------------------------------------------------
    // Debounce
    // --------------------------------------------------------

    rawState = digitalRead(BUTTON_PIN);

    if (rawState != buttonLastRawState)
    {
        buttonLastRawState = rawState;
        buttonLastChangeTime = millis();
    }

    // Has the signal remained stable long enough?
    if (rawState != buttonStableState)
    {
        if (millis() - buttonLastChangeTime >= BUTTON_DEBOUNCE_MS)
        {
            buttonStableState = rawState;

            // ------------------------------------------------
            // Button pressed
            // ------------------------------------------------

            if (buttonStableState == LOW)
            {
                pressStartTime = millis();

                Serial.println("Button pressed");
            }

            // ------------------------------------------------
            // Button released
            // ------------------------------------------------

            else
            {
                if (pressStartTime != 0)
                {
                    uint32_t pressDuration =
                        millis() - pressStartTime;

                    pressStartTime = 0;

                    Serial.print("Button duration: ");
                    Serial.print(pressDuration);
                    Serial.println(" ms");

                    // ========================================
                    // Noise
                    // ========================================

                    if (pressDuration < BUTTON_NOISE_MS)
                    {
                        Serial.println("Noise");
                        return;
                    }

                    // ========================================
                    // 20 second reset
                    // ========================================

                    if (pressDuration >= BUTTON_RESET_MS)
                    {
                        Serial.println("20 second press");

                        Log("===Button===");
                        Log("20sec Press");

                        ResetSchedules();
                        removeFile(SCHEDULE_FILE);
                        removeFile(LOG_FILE);

                        //systemtimestamp = 0;

                        setLEDInterval(100);
                        resetLEDInterval();

                        return;
                    }

                    // ========================================
                    // 10-15 second WiFi reset
                    // ========================================

                    if (
                        pressDuration >= BUTTON_WIFI_MS &&
                        pressDuration <= BUTTON_WIFI_MAX_MS
                    )
                    {
                        Serial.println("10 second press");

                        Log("===Button===");
                        Log("10sec Press");

                        removeFile(WIFICONFIG_FILE);
                        
                        setLEDInterval(300);
                        resetLEDInterval();

                        return;
                    }

                    // ========================================
                    // Normal short press
                    // ========================================

                    Serial.println("Short press");
                }
            }
        }
    }
}