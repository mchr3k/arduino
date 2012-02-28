#include <avr/interrupt.h>
#include <MANCHESTER.h>

const int MAX_NODE_ID = 31;
unsigned int msgNum[MAX_NODE_ID];
const int MAX_MSG_NUM = 31;

unsigned char databufA[5];
unsigned char databufB[5];
unsigned char* currentBuf = databufA;

void setup()
{
 Serial.begin(9600);
 
 MANRX_SetRxPin(4);
 MANRX_SetupReceive();
 MANRX_BeginReceiveBytes(5, currentBuf);
 
 // Zero msg num array
 for (int i = 0; i < MAX_NODE_ID; i++)
 {
   msgNum[i] = 0;
 }
}//end of setup

unsigned int xoData;
unsigned int xoNodeID;

void loop() 
{
  readMsg();
  Serial.print("Read data from node ");
  Serial.print(xoNodeID);
  Serial.print(": ");
  float temp = ((float)xoData) / 10.0;
  Serial.println(temp);
}

void readMsg()
{
  while(true)
  {
    // No idea why this delay is needed
    delay(1);
    if (MANRX_ReceiveComplete())
    {      
      unsigned char receivedSize;
      unsigned char* msgData;
      MANRX_GetMessageBytes(&receivedSize, &msgData);

      // Prepare for next msg
      if (currentBuf == databufA)
      {
        currentBuf = databufB;
      }
      else
      {
        currentBuf = databufA;
      }      
      MANRX_BeginReceiveBytes(5, currentBuf);
    
      // Check if we have a valid message
      // We expect to receive the following
      // 5 bit node ID
      // 5 bit reading number
      // 6 bit unused
      // 16 bit data
      // 8 bit checksum
      
      // This is a total of 5 unsigned chars
      xoNodeID = (msgData[0] & (0b11111 << 3)) >> 3;
      unsigned int thisMsgNum = ((msgData[0] & 0b111) << 3) | 
                                ((msgData[1] & 0b11000000) >> 6);      
      unsigned char checksum = msgData[0] | msgData[1] | 
                               msgData[2] | msgData[3];
      // Ignore data which fails the checksum
      if (checksum != msgData[4]) continue;
      // Ignore duplicates
      if (msgNum[xoNodeID] == thisMsgNum) continue;
      msgNum[xoNodeID] = thisMsgNum;
      // Build up return data
      xoData = (msgData[2] << 8) | msgData[3];
      return;
    }    
  }
}

