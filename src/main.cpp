#include <Arduino.h>

#include <Webserver.h>
#include <RTC.h>

// put function declarations here:


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
	RTC_setTimestamp(systemtimestamp);
	WebserverSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  RTC_cc();
	WebserverHandleClients();
}

// put function definitions here:
