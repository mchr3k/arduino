#ifndef STRREAD_H
#define STRREAD_H

#include <Print.h>
#include "MultiSerial.h"

class StringReader
{
  public:
    int readString(MultiSerial*, char*, int);
};

#endif

