#include <SoftwareSerial.h>
#include "StringReader.h"

SoftwareSerial softSerial(2,3);
StringReader StrReader;

#define SEROUT softSerial

// Buffer which we read data into
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];
#define SERIAL_SPEED 115200

void setup()
{
  softSerial.begin(9600);
  Serial.begin(115200);
}
 
void loop()
{
  while (true)
  {
    int readChars = StrReader.readString(&Serial, stringData, STR_DATA_LEN);
    if (readChars > 0)
    {
      Serial.print("Send to Bluesmirf: ");
      Serial.println(stringData);
      softSerial.println(stringData);
      readChars = StrReader.readString(&softSerial, stringData, STR_DATA_LEN);
      if (readChars > 0)
      {
        Serial.print("Response: ");
        Serial.println(stringData);
      }
    }
  }  
}
