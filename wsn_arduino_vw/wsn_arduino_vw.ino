#include <PString.h>
#include <VirtualWireNewTiny.h>

void setup() 
{
  Serial.begin(9600);  
  vw_set_rx_pin(4);
  vw_setup(1000); // Bits per sec
  vw_rx_start(); // Start the receiver PLL running 
}

void loop() 
{  
  char buffer[50];
  PString mystring(buffer, sizeof(buffer));
  
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  //if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    vw_get_message(buf, &buflen);
    int i;
    // Message with a good checksum received, dump HEX
    Serial.print("Str: ");
    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i]);
    }
    Serial.println();
    Serial.print("Hex: ");
    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i], HEX);
    }
    Serial.println();
    Serial.print("Got: ");
    for (i = 0; i < buflen; i++)
    {
      mystring.begin();
      mystring.print(buf[i], BIN);
      int len = mystring.length();
      mystring.begin();
      for (int i = 0; i < (8 - len);i++)
      {
        mystring.print("0");
      }
      mystring.print(buf[i], BIN);
      Serial.print(mystring);
      Serial.print(" ");
    }
    Serial.println();
    Serial.println();
    delay(10 * 1000);
  }
}
