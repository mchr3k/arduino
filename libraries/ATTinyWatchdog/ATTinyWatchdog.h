#ifndef ATTINYUTIL_h
#define ATTINYUTIL_h

#include <avr/sleep.h>
#include <avr/wdt.h>
#include "WProgram.h"

/*
 Utility class for the ATTiny85
 */
class ATTinyWatchdogClass
{
  public:
    ATTinyWatchdogClass();  //the constructor
    void setup(int ii); //setup the watchdog
    void sleep(int waitTime);  // wait using the watchdog timer    
};

extern ATTinyWatchdogClass ATTINYWATCHDOG;

#endif
