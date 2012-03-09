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
  byte lastindex;
  // last msg num: 0-31 (init'd to -1)
  byte lastmsgnum;
} NodeData;

const int MAX_NODE_ID = 2;
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
   nodes[i].lastindex = 255;
   nodes[i].lastmsgnum = 255;
 }
}

// State for reading from the Serial port
SerialReader SerReader;
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];

void loop() 
{
  recordReceivedData();
  if (SerReader.readString(stringData, STR_DATA_LEN) > 0)
  {
    processCommand();
  }
}

boolean debug_live = false;
boolean debug_msgnums = false;

void processCommand()
{
  Serial.print("== received: ");
  Serial.println(stringData);
  
  if (strcmp(stringData, "debug_live_true") == 0)
  {
    debug_live = true;
    Serial.println("== debug_live: enabled");
  }
  else if (strcmp(stringData, "debug_live_false") == 0)
  {
    debug_live = false;
    Serial.println("== debug_live: disabled");
  }
  else if (strcmp(stringData, "debug_msgnums_true") == 0)
  {
    debug_msgnums = true;
    Serial.println("== debug_msgnums: enabled");
  }
  else if (strcmp(stringData, "debug_msgnums_false") == 0)
  {
    debug_msgnums = false;
    Serial.println("== debug_msgnums: disabled");
  }
  else
  {
    for (int i = 0; i < MAX_NODE_ID; i++)
    {
      if (nodes[i].lastindex == 255)
      {
        Serial.print("== node: ");
        Serial.println(i);
        Serial.println("no_data");
      }
      else
      {
        Serial.print("== node:");
        Serial.println(i);
  
        byte index = nodes[i].lastindex;
        
        for (int j = 0; j <= index; j++)
        {
          unsigned long time = nodes[i].times[j];
          unsigned int reading = nodes[i].readings[j];
          float temp = ((float)reading) / 10.0;
          Serial.print(temp);
          Serial.print(",");
          Serial.print(time);
          if (j < index)
          {
            Serial.print(",");
          }
          else
          {
            Serial.println("");
          }
        } // end reading loop
      } // end node data output
    } // end node loop
  } // end all node output
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
    byte nodeID = (byte)((msgData[0] & (0b11111 << 3)) >> 3);
    byte thisMsgNum = (byte)(((msgData[0] & 0b111) << 2) | 
                            ((msgData[1] & 0b11000000) >> 6));      
    unsigned char checksum = msgData[0] | msgData[1] | 
                             msgData[2] | msgData[3];
    // Ignore data which fails the checksum
    if (checksum != msgData[4]) return;
    // Ignore duplicates
    if (nodes[nodeID].lastmsgnum == thisMsgNum)
    {
      if (debug_msgnums)
      {
        Serial.print("== debug_msgnums retransmit from node: ");
        Serial.println((int)nodeID);
      }
      return;
    }
    
    if (debug_msgnums)
    {
      byte lastnum = nodes[nodeID].lastmsgnum;
      
      if (lastnum == 255)
      {
        Serial.print("== debug_msgnums first msgnum from node: ");
        Serial.println((int)nodeID);
      }
      else
      {
        byte expectednum = lastnum++;
        expectednum++;
        if (expectednum >= 32)
        {
          expectednum = 0;
        }
  
        if (expectednum != thisMsgNum)
        {
          Serial.print("== debug_msgnums unexpected msgnum from node: ");
          Serial.println((int)nodeID);
          Serial.print("expected: ");
          Serial.println((int)expectednum);
          Serial.print("received: ");
          Serial.println((int)thisMsgNum);
        }
      }
    }
    
    nodes[nodeID].lastmsgnum = thisMsgNum;
    // Record reading
    nodes[nodeID].lastindex++;   
    if (nodes[nodeID].lastindex > 11)
    {
      nodes[nodeID].lastindex = 0;
    }
    byte index = nodes[nodeID].lastindex;
    unsigned int reading = (msgData[2] << 8) | msgData[3];
    unsigned long time = millis();
    nodes[nodeID].readings[index] = reading;
    nodes[nodeID].times[index] = time;

    if (debug_live)
    {
      Serial.print("== debug_live reading from node: ");
      Serial.println((int)nodeID);
      float temp = ((float)reading) / 10.0;
      Serial.print(temp);
      Serial.print(",");
      Serial.println(time);
    }
  }
}
