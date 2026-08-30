#include <Arduino.h>

#include <Webserver.h>
#include <RTC.h>
#include <savefile.h>
#include <scheduler.h>
#include <button.h>

// put function declarations here:


void setup() {
	Serial.begin(115200);
	Serial.println();
  Serial.println("Turned On");

  // put your setup code here, to run once:

  /* Setup built-in LED */
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  LoadFiles();
  RTC_Begin();
  SysTS_RTC();
	WebserverSetup();
  ButtonInit();
}

void loop() {
  // put your main code here, to run repeatedly:
  incrementSysTimestamp();
	WebserverHandleClients();
  ProcessSchedules();
  ButtonTask();
}

// put function definitions here:
