#include "MANCHESTER.h"

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)

#define TxPin 4  //the digital pin to use to transmit data

unsigned int Tdata = 0;  //the 16 bits to send

void setup() 
{                
  MANCHESTER.SetTxPin(TxPin);      // sets the digital pin as output default 4
}//end of setup

void loop() 
{
 Tdata +=1;
 MANCHESTER.Transmit(Tdata);
 delay(100);
}//end of loop
