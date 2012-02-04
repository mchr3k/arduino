#include <avr/interrupt.h>
#include <MANCHESTER.h>

const int MSG_SIZE = 3;
const int MAX_NODE_ID = 31;
unsigned int msgNum[MAX_NODE_ID];
const int MAX_MSG_NUM = 31;

void setup()
{   
 Serial.begin(9600);
 
 MANRX_SetupReceive();
 MANRX_BeginReceive();
 
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
  Serial.println(xoData);
}

unsigned int inData[MSG_SIZE];
void readMsg()
{
  int offset = -1;
  int count = 0;
  while(true)
  {
    // No idea why this delay is needed
    delay(1);
    if (MANRX_ReceiveComplete())
    {
      offset++;
      if (offset >= MSG_SIZE) offset = 0;
      
      unsigned int data = MANRX_GetMessage();
      MANRX_BeginReceive();
      
      if (count < MSG_SIZE) count++;
      
      inData[offset] = data;
      
      if (count >= MSG_SIZE)
      {              
        // Copy the last MSG_SIZE worth of data into a working buffer
        int readOffset = offset;
        unsigned int msgData[MSG_SIZE];
        for (int i = (MSG_SIZE  - 1); i >= 0; i--)
        {
          msgData[i] = inData[readOffset];
          readOffset--;
          if (readOffset < 0) readOffset = (MSG_SIZE - 1);
        }
      
        // Check if we have a valid message
        // We expect to receive the following
        // 6 bits pre-amble
        // 5 bit node ID
        // 5 bit reading number
        // 16 bit data
        // 16 bit data (repeated)
        //
        // This is a total of 3x unsigned ints     
        unsigned int preamble = (msgData[0] & (0b111111 << 10)) >> 10;
        if (preamble != 0b010101) continue;
        xoNodeID = (msgData[0] & (0b11111 << 5)) >> 5;
        unsigned int thisMsgNum = msgData[0] & 0b11111;
        if (msgData[1] != msgData[2]) continue;
        // Ignore duplicates
        if (msgNum[xoNodeID] == thisMsgNum) continue;
        msgNum[xoNodeID] = thisMsgNum;
        
        // Got a valid message!
        xoData = msgData[1];
        return;
      }
    }    
  }
}

