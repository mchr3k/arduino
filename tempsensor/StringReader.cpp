#include <Arduino.h>
#include "StringReader.h"

const int STR_DATA_LEN = 50;
const unsigned long MAX_SERIAL_WAIT = 100;

int StringReader::readString(MultiSerial* input, char* stringData, int stringDataLen)
{
  char inChar=-1;
  int strIndex = 0;
  
  // Start at 0 so that we return quickly if there is no data initially
  unsigned long startTime = 0;
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
  
  if (strIndex > 0)
  {
    input->print("Read: ");
    input->println(stringData);
  }
  
  return strIndex;
}
