#include <avr/pgmspace.h>
#include <SerialReader.h>
#include <SD.h>

#define WsnPrint(x) WSNSerialPrint_P(PSTR(x))
#define WsnPrintln(x) WSNSerialPrintln_P(PSTR(x))

static NOINLINE void WSNSerialPrint_P(PGM_P str) 
{
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}

static NOINLINE void WSNSerialPrintln_P(PGM_P str) 
{
  WSNSerialPrint_P(str);
  Serial.println();
}

// SD vars
const int chipSelect = 10;

void setup()
{
  Serial.begin(9600);

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    WsnPrintln("CardFail");
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
}

void processCommand()
{
  WsnPrint("== rcv: ");
  Serial.println(stringData);

  if (strcmp(stringData, "ls") == 0)
  {
    // list all files in the card with date and size
    WsnPrintln("== printing files in /");
    printRoot();
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
    deleteAllFilesFromRoot();
  }
  else
  {
    WsnPrintln("== commands");
    WsnPrintln("help");
    WsnPrintln("dbg_newfile");
    WsnPrintln("ls");
    WsnPrintln("rm *");
  }
  WsnPrintln("== done");
}

void printRoot() 
{
  File root = SD.open("/");
  while(true) 
  {     
    File entry = root.openNextFile();
    if (!entry) 
    {
      // no more files
      break;
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) 
    {
      WsnPrintln("/");
    } 
    else 
    {
      WsnPrint("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  root.close();
}

void deleteAllFilesFromRoot() 
{
  File root = SD.open("/");
  while(true)
  {
    File entry = root.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    SD.remove(entry.name());
    entry.close();
  }
  root.close();
}

void writeNodeFile(byte nodeID)
{
  // the logging file
  File logfile;

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

    if (!SD.exists(filename)) 
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }

  WsnPrint("Writing to file: ");
  Serial.println(filename);

  // Select data to write
  logfile.print("Node ID: ");
  logfile.println((int)nodeID);
  logfile.println("time, temp, ");

  logfile.close();
}
