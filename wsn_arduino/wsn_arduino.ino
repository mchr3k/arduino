#include <PString.h>
#include <MANCHESTER.h>

#define RxPin 4

const int MSG_SIZE = 3;
const int MAX_NODE_ID = 31;
unsigned int msgNum[MAX_NODE_ID];
const int MAX_MSG_NUM = 31;

void setup() 
{ 
 MANCHESTER.SetRxPin(RxPin); //user sets rx pin default 4
 MANCHESTER.SetTimeOut(1000); //user sets timeout default blocks
 Serial.begin(9600);	// Debugging only
 
 // Zero msg num array
 for (int i = 0; i < MAX_NODE_ID; i++)
 {
   msgNum[i] = 0;
 }
}//end of setup

unsigned int xoData;
unsigned int xoNodeID;

char buffer[50];
PString mystring(buffer, sizeof(buffer));

void loop() 
{
  unsigned int data = MANCHESTER.Receive();
    Serial.print("Got: ");
    mystring.begin();
    mystring.print(data, BIN);
    int len = mystring.length();
    mystring.begin();
    for (int i = 0; i < (16 - len);i++)
    {
      mystring.print("0");
    }
    mystring.print(data, BIN);
    Serial.print(mystring);
    Serial.println();
  /*
  readMsg();
  Serial.print("Read data from node ");
  Serial.print(xoNodeID);
  Serial.print(": ");
  Serial.println(xoData);  
  */
}

void readMsg()
{

  unsigned int inData[MSG_SIZE];
  int offset = -1;
  int count = 0;
  while(1)
  {          
    offset++;
    if (offset >= MSG_SIZE) offset = 0;
    
    unsigned int data = MANCHESTER.Receive();
    if (!MANCHESTER.ReceivedTimeout())
    {
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

