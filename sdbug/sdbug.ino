#include <SerialReader.h>
#include <SdFat.h>
#include <SdFatUtil.h>

// SD vars
const int chipSelect = 10;
SdFat sd;
#define error(s) sd.errorHalt_P(PSTR(s))

void setup()
{
  Serial.begin(9600);
  Serial.println("***reset***");

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
  if (SerReader.readString(stringData, STR_DATA_LEN) > 0)
  {
    processCommand();
  }
}

void processCommand()
{
  Serial.print("== rcv: ");
  Serial.println(stringData);

  if (strcmp(stringData, "ls") == 0)
  {
    // list all files in the card with date and size
    Serial.println("== printing files in /");
    listFiles(LS_SIZE);
  }
  else if (strcmp(stringData, "dbg_newfile") == 0)
  {
    Serial.println("== creating file...");
    writeNodeFile(0);
  }
  else if (strcmp(stringData, "rm *") == 0)
  {
    // list all files in the card with date and size
    Serial.println("== deleting all files in /");
    deleteFiles(sd.vwd());
  }
  else
  {
    Serial.println("== commands");
    Serial.println("help");
    Serial.println("dbg_newfile");
    Serial.println("ls");
    Serial.println("rm *");
  }
  Serial.println("== done");
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

/*
 * remove all files in dir.
 */
void deleteFiles(SdBaseFile* dir) 
{
  dir_t p;
  char name[13];
  SdFile file;
  
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

      if (!file.open(dir, name, O_WRITE)) return;
      if (!file.remove()) error("file.remove failed");
    }
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

  Serial.print("Writing to file: ");
  Serial.println(filename);

  // Select data to write
  logfile.print("Node ID: ");
  logfile.println((int)nodeID);
  logfile.println("time, temp, ");

  logfile.close();
}
