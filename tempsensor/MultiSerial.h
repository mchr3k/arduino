#ifndef MULTISER_H
#define MULTISER_H

#include <inttypes.h>
#include <Print.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

class MultiSerial : public Print
{
  private:
    SoftwareSerial softSerial;
    bool readHardwareSerial;
  public:
    MultiSerial(bool);
    void begin(unsigned long);
    virtual int available();
    virtual int read();
    virtual size_t write(uint8_t);
    virtual void flush(void);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

#endif

