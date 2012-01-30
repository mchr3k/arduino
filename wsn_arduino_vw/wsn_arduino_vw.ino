#include <VirtualWireNewTiny.h>

void setup() 
{
  vw_setup(1000); // Bits per sec
  vw_set_rx_pin(4);
  vw_rx_start(); // Start the receiver PLL running 
}

void loop() 
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    // Message with a good checksum received, dump HEX
    Serial.print("Got: ");
    for (i = 0; i < buflen; i++)
    {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
}
