#include <Arduino.h>
#include "SerialReader.h"

// (1000 ms / 9600 (bits/sec)) * 8 (bits/byte) = 0.83
// To be safe we allow 2ms to elapse.
const unsigned long MAX_SERIAL_WAIT = 2;

int SerialReader::readString(char* stringData, int stringDataLen)
{
  char inChar=-1;
  int strIndex = 0;
  
  // Set timeout time initially in the past
  // This means we "time out" immediately unless we get some data
  unsigned long timeoutTime = 0;
  strIndex = 0;
  stringData[0] = '\0';
  
  while(true)
  {
    if (Serial.available() > 0)
    {
      timeoutTime = millis() + MAX_SERIAL_WAIT;
      inChar = Serial.read();
      stringData[strIndex] = inChar;
      strIndex++;
      stringData[strIndex] = '\0';
    }
    
    if ((millis() > timeoutTime) ||
        (strIndex == (stringDataLen - 1)))
    {
      break;
    }
  }

#ifdef SERIALREAD_DEBUG
  if (strIndex > 0)
  {
    Serial.print("Read: ");
    Serial.println(stringData);
  }
#endif
  
  return strIndex;
}
