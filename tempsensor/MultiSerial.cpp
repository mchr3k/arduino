#include "MultiSerial.h"

MultiSerial::MultiSerial (bool readHardware) : readHardwareSerial(readHardware), softSerial(2,3) {}

void MultiSerial::begin(unsigned long serSpeed)
{
  Serial.begin(serSpeed);
  //softSerial.begin(serSpeed);
}
int MultiSerial::available()
{
  if (readHardwareSerial)
  {
    return Serial.available();
  }
  else
  {
    return softSerial.available();
  }
}
int MultiSerial::read()
{
  if (readHardwareSerial)
  {
    return Serial.read();
  }
  else
  {
    return softSerial.read();
  }
}
size_t MultiSerial::write(uint8_t c)
{
  //return softSerial.write(c);
  return Serial.write(c);
}
void MultiSerial::flush()
{
  Serial.flush();
}
