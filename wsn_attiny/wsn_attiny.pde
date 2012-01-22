#include "MANCHESTER.h"

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)

#define TxPin 4  //the digital pin to use to transmit data

unsigned int Tdata = 0;  //the 16 bits to send

const int NODE_ID = 1;

void setup() 
{   
  randomSeed(analogRead(0));  
  MANCHESTER.SetTxPin(TxPin);      // sets the digital pin as output default 4
}//end of setup

void loop() 
{
 Tdata +=1;
 sendMsg(Tdata);
 delay(500);
}//end of loop

unsigned int readingNum = 1;

void sendMsg(unsigned int data)
{
  readingNum++;
  if (readingNum >= 31) readingNum = 0;
  doSendMsg(data, readingNum);
  delay(500 + random(500));
  doSendMsg(data, readingNum);
  delay(500 + random(500));
  doSendMsg(data, readingNum);
}

void doSendMsg(unsigned int data, unsigned int msgNum)
{       
  // Send a message with the following format
  // 6 bits pre-amble
  // 5 bit node ID
  // 5 bit reading number
  // 16 bit data
  // 16 bit data (repeated)
  //
  // This is a total of 3x unsigned ints     
  unsigned int preamble = (0b010101 << 10);
  unsigned int nodeID = ((NODE_ID & 0b11111) << 5);
  unsigned int firstByte = preamble | nodeID | (msgNum & 0b11111);
  
  MANCHESTER.Transmit(firstByte);
  MANCHESTER.Transmit(data);
  MANCHESTER.Transmit(data);
}

