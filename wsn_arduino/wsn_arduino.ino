#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <MANCHESTER.h>
#include <SerialReader.h>
#include <SD.h>

const prog_uchar startupHelp1Message[] PROGMEM  = {"Hold  [learn] at power on to display stored cards."};

void PROGMEMprint(const prog_uchar str[])
{
  char c;
  if(!str) return;
  while((c = pgm_read_byte(str++)))
    Serial.print((char)c);
}

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

// Per node data
const int MAX_NODE_ID = 2;
NodeData nodes[MAX_NODE_ID];

// Receive buffers used for RF message reception
unsigned char databufA[5];
unsigned char databufB[5];
unsigned char* currentBuf = databufA;

// The configured time offset
unsigned long timeoffset = 0;

// SD vars
const int chipSelect = 10;
File root;

void setup()
{
 Serial.begin(9600);
 
 // Start receiving data
 MANRX_SetRxPin(4);
 MANRX_SetupReceive();
 
 // Prepare data structures
 for (int i = 0; i < MAX_NODE_ID; i++)
 { 
   nodes[i].lastindex = 255;
   nodes[i].lastmsgnum = 255;
 }
 
 // make sure that the default chip select pin is set to
 // output, even if you don't use it:
 pinMode(10, OUTPUT);
 
 // see if the card is present and can be initialized:
 if (!SD.begin(chipSelect)) {
   Serial.println("CardFail");
   // don't do anything more:
   return;
 }
 
 Serial.println("CardInit"); 
 root = SD.open("/");
 Serial.println("GotRoot");
 
 PROGMEMprint(startupHelp1Message);
}

// State for reading from the Serial port
SerialReader SerReader;
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];

void loop() 
{
  //recordReceivedData();
  //recordTestData();
  if (SerReader.readString(stringData, STR_DATA_LEN) > 0)
  {
    processCommand();
  }
}

boolean debug_live = true;
boolean debug_msgnums = true;
boolean debug_files = true;

void processCommand()
{
  Serial.print("== rcv: ");
  Serial.println(stringData);
  
  if (strcmp(stringData, "dbg_live_t") == 0)
  {
    debug_live = true;
    Serial.println("== dbg_live: t");
  }
  else if (strcmp(stringData, "dbg_live_f") == 0)
  {
    debug_live = false;
    Serial.println("== dbg_live: f");
  }
  else if (strcmp(stringData, "dbg_msgs_t") == 0)
  {
    debug_msgnums = true;
    Serial.println("== dbg_msgs: t");
  }
  else if (strcmp(stringData, "dbg_msgs_f") == 0)
  {
    debug_msgnums = false;
    Serial.println("== dbg_msgs: f");
  }
  else if (strcmp(stringData, "dbg_fls_t") == 0)
  {
    debug_files = true;
    Serial.println("== dbg_fls: r");
  }
  else if (strcmp(stringData, "dbg_fls_f") == 0)
  {
    debug_files = false;
    Serial.println("== dbg_fls: f");
  }
  else if (strncmp(stringData, "set_time_", 9) == 0)
  {
    unsigned long realreftime = (unsigned long)atol((stringData + 9));
    unsigned long localreftime = millis();
    
    // Setting time for the first time
    if (timeoffset == 0)
    {
      MANRX_BeginReceiveBytes(5, currentBuf);
    }
    
    timeoffset = (realreftime - localreftime);
    
    Serial.println("== set_time:");
    Serial.print(" - real   : ");
    Serial.println(realreftime);
    Serial.print(" - local  : ");
    Serial.println(localreftime);
    Serial.print(" - offset : ");
    Serial.println(timeoffset);
  }
  else if (strcmp(stringData, "ls") == 0)
  {
    // list all files in the card with date and size
    Serial.println("== ");
    root = SD.open("/");
    printDirectory(root, 0);
    Serial.println("== ");
  }
  else
  {
    if (timeoffset == 0)
    {
      Serial.println("time_offset: not_set");
    }
    else
    {
      printAllNodes();
    }
  } // end all node output
}

void printDirectory(File dir, int numTabs) 
{
   while(true) 
   {     
     File entry =  dir.openNextFile();
     if (! entry) 
     {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) 
     {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } 
     else 
     {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
   }
}

void printAllNodes()
{
  for (int i = 0; i < MAX_NODE_ID; i++)
  {
    Serial.print("== node_data: ");
    Serial.print("Node ID: ");
    Serial.println(i);
    if (nodes[i].lastindex == 255)
    {
      Serial.println("no_data");
    }
    else
    {
      Serial.println("time, temp, ");
  
      byte index = nodes[i].lastindex;      
      for (int j = 0; j <= index; j++)
      {
        unsigned long time = nodes[i].times[j];
        unsigned int reading = nodes[i].readings[j];
        float temp = ((float)reading) / 10.0;
        Serial.print(time);
        Serial.print(",");
        Serial.println(temp);
      } // end reading loop
    } // end node data output
  } // end node loop
}

unsigned int counter = 0;
unsigned long lasttime = 0;

void recordTestData()
{
  unsigned long elapsed = millis() - lasttime;
  if (elapsed > 1000)
  {    
    lasttime = millis();
    counter++;
    
    addData(0, 0, counter, millis());
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
        Serial.print("== dbg_msgs: retrans, node: ");
        Serial.println((int)nodeID);
      }
      return;
    }
    
    if (debug_msgnums)
    {
      byte lastnum = nodes[nodeID].lastmsgnum;
      
      if (lastnum == 255)
      {
        Serial.print("== dbg_msgs 1st msg, node: ");
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
          Serial.print("== dbg_msgs err msgnum, node: ");
          Serial.println((int)nodeID);
          Serial.print("exp'd: ");
          Serial.println((int)expectednum);
          Serial.print("recv'd: ");
          Serial.println((int)thisMsgNum);
        }
      }
    }
    
    unsigned int reading = (msgData[2] << 8) | msgData[3];
    unsigned long time = millis() + timeoffset;
    
    if (debug_live)
    {
      Serial.print("== dbg_live reading, node: ");
      Serial.println((int)nodeID);
      float temp = ((float)reading) / 10.0;
      Serial.print(temp);
      Serial.print(",");
      Serial.println(time);
    }
    
    // Add data reading
    addData(nodeID, thisMsgNum, reading, time);
    
    if (nodes[nodeID].lastindex >= 11)
    {
      // Write data to file
      writeNodeFile(nodeID);
      
      // Prepare for new data
      nodes[nodeID].lastindex = 0;
    }
  }
}

void addData(byte nodeID, byte thisMsgNum, 
             unsigned int reading,
             unsigned long time)
{
  nodes[nodeID].lastmsgnum = thisMsgNum;
  nodes[nodeID].lastindex++;  
  byte index = nodes[nodeID].lastindex;
  nodes[nodeID].readings[index] = reading;
  nodes[nodeID].times[index] = time;
  
  if (nodes[nodeID].lastindex >= 11)
  {
    // Write data to file
    writeNodeFile(nodeID);
    
    // Prepare for new data
    nodes[nodeID].lastindex = 0;
  }
}

void writeNodeFile(byte nodeID)
{
  // the logging file
  File logfile;
  
  // create a new file
  char filename[] = "00000000.CSV";
  
  // first two chars are the node ID
  sprintf(&filename[0], "%02d", (int)nodeID);
  
  // next 6 chars are a log ID. We expect 1
  // log file per hour so 6 chars allows 999999
  // log files. 24/day gives 114 years of log files.
  for (int i = 0; i < 1000000; i++) 
  {
    sprintf(&filename[2], "%06d", i);

    Serial.print("TestFile: ");
    Serial.println(filename);    
    if (!SD.exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (debug_files)
  {
    Serial.print("WriteFile: ");
    Serial.println(filename);
  }
  
  // Select data to write
  NodeData data = nodes[nodeID];
  logfile.print("Node ID: ");
  logfile.println((int)nodeID);
  logfile.println("time, temp, ");
  
  byte index = data.lastindex;
  for (int j = 0; j <= index; j++)
  {
    unsigned long time = data.times[j];
    unsigned int reading = data.readings[j];
    float temp = ((float)reading) / 10.0;
    logfile.print(time);
    logfile.print(",");
    logfile.println(temp);
  } // end writing loop
  
  logfile.flush();
  logfile.close();
}
