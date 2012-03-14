#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <MANCHESTER.h>
#include <SerialReader.h>
#include <SdFat.h>
#include <SdFatUtil.h>

#define WsnPrint(x) WSNSerialPrint_P(PSTR(x))
#define WsnPrintln(x) WSNSerialPrintln_P(PSTR(x))

static void WSNSerialPrint_P(PGM_P str) 
{
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}

static void WSNSerialPrintln_P(PGM_P str) 
{
  WSNSerialPrint_P(str);
  Serial.println();
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
} 
NodeData;

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
SdFat sd;
#define error(s) sd.errorHalt_P(PSTR(s))

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
  if (!sd.init(SPI_HALF_SPEED)) sd.initErrorHalt();
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
  WsnPrint("== rcv: ");
  Serial.println(stringData);

  if (strcmp(stringData, "help") == 0)
  {
    WsnPrintln("== commands");
    WsnPrintln("help");
    WsnPrintln("status");
    WsnPrintln("dbg_live_t/f");
    WsnPrintln("dbg_msgs_t/f");
    WsnPrintln("dbg_fls_t/f");
    WsnPrintln("dbg_ram");
    WsnPrintln("dbg_newfile");
    WsnPrintln("set_time_<n>");
    WsnPrintln("ls");
    WsnPrintln("rm *");
    WsnPrintln("else: show latest data");
  }
  else if (strcmp(stringData, "status") == 0)
  {
    WsnPrintln("== status");
    WsnPrint("dbg_live: ");
    Serial.println(debug_live);
    WsnPrint("dbg_msgs: ");
    Serial.println(debug_msgnums);
    WsnPrint("dbg_fls: ");
    Serial.println(debug_files);
  }
  else if (strcmp(stringData, "dbg_live_t") == 0)
  {
    debug_live = true;
    WsnPrintln("== dbg_live: t");
  }
  else if (strcmp(stringData, "dbg_live_f") == 0)
  {
    debug_live = false;
    WsnPrintln("== dbg_live: f");
  }
  else if (strcmp(stringData, "dbg_msgs_t") == 0)
  {
    debug_msgnums = true;
    WsnPrintln("== dbg_msgs: t");
  }
  else if (strcmp(stringData, "dbg_msgs_f") == 0)
  {
    debug_msgnums = false;
    WsnPrintln("== dbg_msgs: f");
  }
  else if (strcmp(stringData, "dbg_fls_t") == 0)
  {
    debug_files = true;
    WsnPrintln("== dbg_fls: r");
  }
  else if (strcmp(stringData, "dbg_fls_f") == 0)
  {
    debug_files = false;
    WsnPrintln("== dbg_fls: f");
  }
  else if (strcmp(stringData, "dbg_ram") == 0)
  {
    WsnPrint("Free RAM: ");
    Serial.println(FreeRam());
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

    WsnPrintln("== set_time:");
    WsnPrint(" - real   : ");
    Serial.println(realreftime);
    WsnPrint(" - local  : ");
    Serial.println(localreftime);
    WsnPrint(" - offset : ");
    Serial.println(timeoffset);
  }
  else if (strcmp(stringData, "ls") == 0)
  {
    // list all files in the card with date and size
    WsnPrintln("== printing files in /");
    listFiles(LS_SIZE);
  }
  else if (strcmp(stringData, "dbg_newfile") == 0)
  {
    WsnPrintln("== creating file...");
    writeNodeFile(0);
  }
  else if (strcmp(stringData, "rm *") == 0)
  {
    // list all files in the card with date and size
    WsnPrintln("== deleting all files in /");
    deleteFiles(sd.vwd());
  }
  else
  {
    if (timeoffset == 0)
    {
      WsnPrintln("time_offset: not_set");
    }
    else
    {
      printAllNodes();
    }
  } // command selection
  WsnPrintln("== done");
}

void listFiles(uint8_t flags)
{
  dir_t p;
  
  SdBaseFile* root = sd.vwd();
  root->rewind();
  
  while (root->readDir(&p) > 0) 
  {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break;
 
    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;
 
    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) continue;
 
    // print file name with possible blank fill
    for (uint8_t i = 0; i < 11; i++) 
    {
      if (p.name[i] == ' ') continue;
      if (i == 8) 
      {
        Serial.print('.');
      }
      Serial.write(p.name[i]);
    }
    if (DIR_IS_SUBDIR(&p)) 
    {
      Serial.print('/');
    }
 
    // print modify date/time if requested
    if (flags & LS_DATE) 
    {
       root->printFatDate(p.lastWriteDate);
       Serial.print(" ");
       root->printFatTime(p.lastWriteTime);
    }
    
    // print size if requested
    if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) 
    {
      Serial.print(" ");
      Serial.print(p.fileSize);
    }
    Serial.println();
  }
}

void deleteFiles(SdBaseFile* root) 
{
  dir_t p;
  char name[13];
  SdFile file;
  
  root->rewind();
  
  while (root->readDir(&p) > 0) 
  {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break; 
    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;
    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) continue;
 
    if (DIR_IS_SUBDIR(&p)) 
    {
      // ignore directory
    }
    else
    {
      // print file name with possible blank fill
      uint8_t j = 0;
      for (uint8_t i = 0; i < 11; i++) 
      {
        if (p.name[i] == ' ') continue;
        if (i == 8) 
        {
          sprintf(&name[j], ".");
          j++;
        }
        sprintf(&name[j], "%c", p.name[i]);
        j++;
      }

      if (!file.open(root, name, O_WRITE)) return;
      if (!file.remove()) error("file.remove failed");
    }
  }
}

void printAllNodes()
{
  for (int i = 0; i < MAX_NODE_ID; i++)
  {
    WsnPrintln("== node_data: ");
    WsnPrint("Node ID: ");
    Serial.println(i);
    if (nodes[i].lastindex == 255)
    {
      WsnPrintln("no_data");
    }
    else
    {
      WsnPrintln("time, temp, ");

      byte index = nodes[i].lastindex;      
      for (int j = 0; j <= index; j++)
      {
        unsigned long time = nodes[i].times[j];
        unsigned int reading = nodes[i].readings[j];
        float temp = ((float)reading) / 10.0;
        Serial.print(time);
        WsnPrint(",");
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
  if (elapsed > 5000)
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
        WsnPrint("== dbg_msgs: retrans, node: ");
        Serial.println((int)nodeID);
      }
      return;
    }

    if (debug_msgnums)
    {
      byte lastnum = nodes[nodeID].lastmsgnum;

      if (lastnum == 255)
      {
        WsnPrint("== dbg_msgs 1st msg, node: ");
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
          WsnPrint("== dbg_msgs err msgnum, node: ");
          Serial.println((int)nodeID);
          WsnPrint("exp'd: ");
          Serial.println((int)expectednum);
          WsnPrint("recv'd: ");
          Serial.println((int)thisMsgNum);
        }
      }
    }

    unsigned int reading = (msgData[2] << 8) | msgData[3];
    unsigned long time = millis() + timeoffset;

    if (debug_live)
    {
      WsnPrint("== dbg_live reading, node: ");
      Serial.println((int)nodeID);
      float temp = ((float)reading) / 10.0;
      Serial.print(temp);
      WsnPrint(",");
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

void addData(byte nodeID, 
             byte thisMsgNum, 
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
  SdFile logfile;

  // create a new file
  char filename[] = "/00000000.CSV";

  // first two chars are the node ID
  sprintf(&filename[1], "%02d", (int)nodeID);

  // next 6 chars are a log ID. We expect 1
  // log file per hour so 6 chars allows 999999
  // log files. 24/day gives 114 years of log files.
  for (int i = 0; i < 1000000; i++) 
  {
    sprintf(&filename[3], "%06d", i);
    sprintf(&filename[9], ".CSV");

    if (!sd.vwd()->exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile.open(filename, O_CREAT | O_EXCL | O_WRITE); 
      break;  // leave the loop!
    }
  }

  if (debug_files)
  {
    WsnPrint("Writing to file: ");
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

  logfile.close();
}
