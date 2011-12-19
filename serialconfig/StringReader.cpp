#include <Arduino.h>
#include "StringReader.h"

const int STR_DATA_LEN = 50;
const unsigned long MAX_SERIAL_WAIT = 1000;

int StringReader::readString(Stream* input, char* stringData, int stringDataLen)
{
  char inChar=-1;
  int strIndex = 0;
  
  unsigned long startTime = millis();
  strIndex = 0;
  stringData[0] = '\0';
  
  while(true)
  {
    if (input->available() > 0)
    {
      startTime = millis();
      inChar = input->read();
      stringData[strIndex] = inChar;
      strIndex++;
      stringData[strIndex] = '\0';
    }
    
    unsigned long elapsedTime = millis() - startTime;
    if ((elapsedTime > MAX_SERIAL_WAIT) ||
        (strIndex == (stringDataLen - 1)))
    {
      break;
    }
  }
  
  return strIndex;
}
