#include <avr/interrupt.h>
#include <MANCHESTER.h>
#include <SerialReader.h>

typedef struct
{
  // 1 reading every 5 minutes
  // 12 readings per hour
  unsigned int readings[12];
  // Time of 12 readings per hour
  unsigned long times[12];
  // count of readings: 0-11 (init'd to -1)
  char lastindex;
  // last msg num: 0-31 (init'd to -1)
  char lastmsgnum;
} NodeData;

const int MAX_NODE_ID = 31;
NodeData nodes[MAX_NODE_ID];

unsigned char databufA[5];
unsigned char databufB[5];
unsigned char* currentBuf = databufA;

void setup()
{
 Serial.begin(9600);
 
 MANRX_SetRxPin(4);
 MANRX_SetupReceive();
 MANRX_BeginReceiveBytes(5, currentBuf);
 
 for (int i = 0; i < MAX_NODE_ID; i++)
 { 
   nodes[i].lastindex = -1;
   nodes[i].lastmsgnum = -1;
 }
}

// State for reading from the Serial port
SerialReader SerReader;
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];

void loop() 
{
  if (SerReader.readString(stringData, STR_DATA_LEN) > 0)
  {
    processCommand();
  }
  recordReceivedData();
}

void processCommand()
{
  for (int i = 0; i < MAX_NODE_ID; i++)
  {
    if (nodes[i].lastindex < 0)
    {
      Serial.print("No data from node ID: ");
      Serial.println(i);
    }
    else
    {
      Serial.print("Read data from node ");
      Serial.print(i);
      Serial.print(": ");

      char index = nodes[i].lastindex;
      unsigned int reading = nodes[i].readings[index];
      float temp = ((float)reading) / 10.0;

      Serial.println(temp);
    }
  }  
}

void recordReceivedData()
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
    { currentBuf = databufB; }
    else
    { currentBuf = databufA; }
    
    MANRX_BeginReceiveBytes(5, currentBuf);
  
    // Check if we have a valid message
    // We expect to receive the following
    // 5 bit node ID
    // 5 bit reading number
    // 6 bit unused
    // 16 bit data
    // 8 bit checksum
    
    // This is a total of 5 unsigned chars
    char nodeID = (char)((msgData[0] & (0b11111 << 3)) >> 3);
    char thisMsgNum = (char)(((msgData[0] & 0b111) << 3) | 
                            ((msgData[1] & 0b11000000) >> 6));      
    unsigned char checksum = msgData[0] | msgData[1] | 
                             msgData[2] | msgData[3];
    // Ignore data which fails the checksum
    if (checksum != msgData[4]) return;
    // Ignore duplicates
    if (nodes[nodeID].lastmsgnum == thisMsgNum) return;
    nodes[nodeID].lastmsgnum = thisMsgNum;
    // Record reading
    nodes[nodeID].lastindex++;   
    if (nodes[nodeID].lastindex > 11)
    {
      nodes[nodeID].lastindex = 0;
    }
    char index = nodes[nodeID].lastindex;
    unsigned int reading = (msgData[2] << 8) | msgData[3];
    nodes[nodeID].readings[index] = reading;
    nodes[nodeID].times[index] = millis();
  }
}
