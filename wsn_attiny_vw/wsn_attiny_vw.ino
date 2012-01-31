#include <VirtualWireNewTiny.h>

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)

#define TxPin 4  //the digital pin to use to transmit data

void setup() 
{
  // Pins 3, 4 are used for output
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);  
  // Pin 3 is used as a controlled VCC while we are awake
  digitalWrite(3, HIGH);
  // VW Setup  
  vw_set_tx_pin(4);
  vw_setup(1000);
}

void loop() 
{
  const char *msg = "DDDDDDDDDD";
  vw_send((uint8_t *)msg, strlen(msg));
  delay(5000);
}
