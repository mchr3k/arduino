#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <MANCHESTER.h>
#include <SerialReader.h>
#include <SdFat.h>
#include <SdFatUtil.h>

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
int lastfile[MAX_NODE_ID];

void setup()
{
  Serial.begin(9600);

  // Start receiving data
  MANRX_SetRxPin(4);
  MANRX_SetupReceive();

  // Prepare data structures
  for (int i = 0; i < MAX_NODE_ID; i++)
  { 
    nodes[i].lastindex = 0;
    nodes[i].lastmsgnum = 255;
    lastfile[i] = 0;
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
  recordReceivedData();
  //recordTestData();
  if (SerReader.readString(stringData, STR_DATA_LEN) > 0)
  {
    processCommand();
  }
}

boolean debug_live = false;
boolean debug_msgnums = false;
boolean debug_files = false;

void processCommand()
{
  PgmPrint("== rcv: ");
  Serial.println(stringData);

  if (strcmp_P(stringData, PSTR("help")) == 0)
  {
    PgmPrintln("== commands");
    PgmPrintln("help");
    PgmPrintln("status");
    PgmPrintln("dbg_live_t/f");
    PgmPrintln("dbg_msgs_t/f");
    PgmPrintln("dbg_fls_t/f");
    PgmPrintln("dbg_ram");
    PgmPrintln("dbg_newfile");
    PgmPrintln("set_time_<n>");
    PgmPrintln("ls");
    PgmPrintln("cat <file>");
    PgmPrintln("rm <file>");
    PgmPrintln("rm *");
    PgmPrintln("else: show latest data");
  }
  else if (strcmp_P(stringData, PSTR("status")) == 0)
  {
    PgmPrintln("== status");
    PgmPrint("dbg_live: ");
    Serial.println(debug_live);
    PgmPrint("dbg_msgs: ");
    Serial.println(debug_msgnums);
    PgmPrint("dbg_fls: ");
    Serial.println(debug_files);
  }
  else if (strcmp_P(stringData, PSTR("dbg_live_t")) == 0)
  {
    debug_live = true;
    PgmPrintln("== dbg_live: t");
  }
  else if (strcmp_P(stringData, PSTR("dbg_live_f")) == 0)
  {
    debug_live = false;
    PgmPrintln("== dbg_live: f");
  }
  else if (strcmp_P(stringData, PSTR("dbg_msgs_t")) == 0)
  {
    debug_msgnums = true;
    PgmPrintln("== dbg_msgs: t");
  }
  else if (strcmp_P(stringData, PSTR("dbg_msgs_f")) == 0)
  {
    debug_msgnums = false;
    PgmPrintln("== dbg_msgs: f");
  }
  else if (strcmp_P(stringData, PSTR("dbg_fls_t")) == 0)
  {
    debug_files = true;
    PgmPrintln("== dbg_fls: t");
  }
  else if (strcmp_P(stringData, PSTR("dbg_fls_f")) == 0)
  {
    debug_files = false;
    PgmPrintln("== dbg_fls: f");
  }
  else if (strcmp_P(stringData, PSTR("dbg_ram")) == 0)
  {
    PgmPrint("Free RAM: ");
    Serial.println(FreeRam());
  }
  else if (strncmp_P(stringData, PSTR("set_time_"), 9) == 0)
  {
    unsigned long realreftime = (unsigned long)atol((stringData + 9));
    unsigned long localreftime = (millis() / 1000);

    // Setting time for the first time
    if (timeoffset == 0)
    {
      MANRX_BeginReceiveBytes(5, currentBuf);
    }

    timeoffset = (realreftime - localreftime);

    PgmPrintln("== set_time:");
    PgmPrint(" - real   : ");
    Serial.println(realreftime);
    PgmPrint(" - local  : ");
    Serial.println(localreftime);
    PgmPrint(" - offset : ");
    Serial.println(timeoffset);
  }
  else if (strcmp_P(stringData, PSTR("ls")) == 0)
  {
    // list all files in the card with date and size
    PgmPrintln("== printing files in /");
    listRootFiles(LS_SIZE);
  }
  else if (strcmp_P(stringData, PSTR("dbg_newfile")) == 0)
  {
    PgmPrintln("== creating file...");
    writeNodeFile(0);
  }
  else if (strncmp_P(stringData, PSTR("cat "), 4) == 0)
  {
    PgmPrintln("== cat file in /");
    catFile(&stringData[4]);
  }
  else if (strncmp_P(stringData, PSTR("rm "), 3) == 0)
  {
    if (stringData[3] == '*')
    {
      PgmPrintln("== deleting all files in /");
      deleteRootFiles();
    }
    else
    {
      PgmPrintln("== deleting file in /");
      deleteFile(&stringData[3]);
    }
  }
  else
  {
    if (timeoffset == 0)
    {
      PgmPrintln("time_offset: not_set");
    }
    else
    {
      printAllNodes();
    }
  } // command selection
  PgmPrintln("== done");
}

void listRootFiles(uint8_t flags)
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

void catFile(char* filename)
{
  SdBaseFile* root = sd.vwd();
  SdFile file;
  
  if (!file.open(root, filename, O_READ)) return;
  
  Serial.print(filename);
  PgmPrintln(":");
  // read from the file until there's nothing else in it:
  int data;
  while ((data = file.read()) > 0) Serial.write(data);
  // close the file:
  file.close();
}

void deleteFile(char* filename)
{
  SdBaseFile* root = sd.vwd();
  SdFile file;
  
  if (!file.open(root, filename, O_WRITE)) return;
  if (!file.remove()) 
  {
    error("file.remove failed");
  }
  else
  {
    PgmPrint("deleted: ");
    Serial.println(filename);
  }
}

void deleteRootFiles() 
{
  SdBaseFile* root = sd.vwd();
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
    PgmPrintln("== node_data: ");
    PgmPrint("Node ID: ");
    Serial.println(i);
    if (nodes[i].lastindex == 0)
    {
      PgmPrintln("no_data");
    }
    else
    {
      PgmPrintln("time, temp, ");

      byte index = nodes[i].lastindex;      
      for (int j = 0; j < index; j++)
      {
        unsigned long time = nodes[i].times[j];
        unsigned int reading = nodes[i].readings[j];
        float temp = ((float)reading) / 10.0;
        Serial.print(time);
        PgmPrint(",");
        Serial.print(temp);
        PgmPrintln(",");
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
    unsigned long time = (millis() / 1000) + timeoffset;
    addData(0, 0, counter, time);
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
        PgmPrint("== dbg_msgs: retrans, node: ");
        Serial.print((int)nodeID);
        PgmPrint(" (msg num: ");
        Serial.print((int)thisMsgNum);
        PgmPrintln(")");
      }
      return;
    }
    else if (debug_msgnums)
    {
      byte lastnum = nodes[nodeID].lastmsgnum;

      if (lastnum == 255)
      {
        PgmPrint("== dbg_msgs 1st msg, node: ");
        Serial.print((int)nodeID);
        PgmPrint(" (msg num: ");
        Serial.print((int)thisMsgNum);
        PgmPrintln(")");
      }
      else
      {
        byte expectednum = lastnum + 1;
        if (expectednum >= 31)
        {
          expectednum = 0;
        }

        if (expectednum != thisMsgNum)
        {
          PgmPrint("== dbg_msgs err msgnum, node: ");
          Serial.println((int)nodeID);
          PgmPrint("exp'd: ");
          Serial.println((int)expectednum);
          PgmPrint("recv'd: ");
          Serial.println((int)thisMsgNum);
        }
      }
    }

    unsigned int reading = (msgData[2] << 8) | msgData[3];
    unsigned long time = (millis() / 1000) + timeoffset;

    if (debug_live)
    {
      PgmPrint("== dbg_live reading, node: ");
      Serial.print((int)nodeID);
      PgmPrint(" (msg num: ");
      Serial.print((int)thisMsgNum);
      PgmPrintln(")");
      float temp = ((float)reading) / 10.0;
      Serial.print(temp);
      PgmPrint(",");
      Serial.println(time);
    }

    // Add data reading
    addData(nodeID, thisMsgNum, reading, time);
  }
}

void addData(byte nodeID, 
             byte thisMsgNum, 
             unsigned int reading,
             unsigned long time)
{
  nodes[nodeID].lastmsgnum = thisMsgNum;  
  byte index = nodes[nodeID].lastindex;
  nodes[nodeID].lastindex++;
  nodes[nodeID].readings[index] = reading;
  nodes[nodeID].times[index] = time;

  if (nodes[nodeID].lastindex > 11)
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
  for (int i = lastfile[nodeID]; i < 1000000; i++) 
  {
    sprintf(&filename[3], "%06d", i);
    sprintf(&filename[9], ".CSV");

    if (!sd.vwd()->exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile.open(filename, O_CREAT | O_EXCL | O_WRITE); 
      lastfile[nodeID] = i + 1;
      break;  // leave the loop!
    }
  }

  if (debug_files)
  {
    PgmPrint("Writing to file: ");
    Serial.println(filename);
  }

  // Select data to write
  NodeData data = nodes[nodeID];
  logfile.print("Node ID: ");
  logfile.println((int)nodeID);
  logfile.println("time, temp, ");

  byte index = data.lastindex;
  for (int j = 0; j < index; j++)
  {
    unsigned long time = data.times[j];
    unsigned int reading = data.readings[j];
    float temp = ((float)reading) / 10.0;
    logfile.print(time);
    logfile.print(",");
    logfile.print(temp);
    logfile.println(",");
  } // end writing loop

  logfile.close();
}
