#include <avr/interrupt.h>

#define RxPin 4

/*
Timer 2 is used with a ATMega328.

This code gives a basic data rate as 1000 bits/s. In manchester encoding we send 1 0 for a data bit 0.
We send 0 1 for a data bit 1. This ensures an average over time of a fixed DC level in the TX/RX.
This is required by the ASK RF link system to ensure its correct operation.
The actual data rate is then 500 bits/s.

  1,000,000 / 1,000 = 1,000uS/bit

Timer 2 in the ATMega328 is used to find the time between each transition coming from the demodulation circuit.
Their setup is for normal count. No connections. No interupts.
The divide ratio is /256 with 16 Mhz the 328.

  1 / (16,000,000 / 256) = 16uS/count
  1,000 / 16 = 62.5 counts/bit

This gives 16 usec per count. For a normal pulse this is 62.5 counts.
With an error allowance of 22.5 usec we get the following:
*/
#define MinCount 40  //pulse lower count limit on capture
#define MaxCount 85  //pulse higher count limit on capture
#define MinLongCount 103  //pulse lower count on double pulse
#define MaxLongCount 147  //pulse higher count on double pulse

static int rx_sample = 0;
static int rx_last_sample = 0;
static uint8_t rx_count = 0;
static uint8_t rx_sync_count = 0;
static uint8_t rx_mode = 0;
static uint8_t rx_max_mode = 0;

unsigned int rx_manBits = 0; //the received manchester 32 bits
unsigned char rx_numMB = 0;  //the number of received manchester bits
boolean rx_startbit = true;  //remember to ignore the start bit is a one

unsigned char rx_maxBytes = 2;
unsigned char rx_curByte = 0;
unsigned char rx_data[2];

boolean rx_debug = false;

void AddManBit(unsigned int *manBits, unsigned char *numMB, 
               unsigned char *curByte, unsigned char *data, 
               unsigned char bit)
{
  *manBits <<= 1;
  *manBits |= bit;
  (*numMB)++;
  if (*numMB == 16)
  {
    unsigned char newData = 0;

    for (char i = 0; i < 8; i++)
    {
      // ManBits holds 16 bits of manchester data
      // 1 = LO,HI
      // 0 = HI,LO
      // We can decode each bit by looking at the bottom bit of each pair.
      newData <<= 1;
      newData |= (*manBits & 1); // store the one
      *manBits = *manBits >> 2; //get next data bit    
    }
    data[*curByte] = newData;
    (*curByte)++;
    *numMB = 0;
  }
}

ISR(TIMER2_COMPA_vect)
{
  // Increment counter  
  rx_count += 5;
  
  // Check for value change
  rx_sample = digitalRead(RxPin);
  boolean transition = (rx_sample != rx_last_sample);

  // Record the max mode we entered
  if (rx_max_mode < rx_mode) rx_max_mode = rx_mode;
  
  if (rx_mode == 0)
  {
    // Wait for first transition to HIGH
    if (transition && (rx_sample == 1))
    {        
      rx_count = 0;
      rx_sync_count = 0;
      rx_mode = 1;
    }
  }
  else if (rx_mode == 1)
  {
    // Initial sync block
    if (transition)
    {
      if(((rx_sync_count < 20) || (rx_sample == 1)) && 
         ((rx_count < MinCount) || (rx_count > MaxCount)))
      {
        // Transition was too slow/fast
        rx_mode = 0;
      }
      else if((rx_sample == 0) &&
              ((rx_count < MinCount) || (rx_count > MaxLongCount)))
      {
        // Transition was too slow/fast
        rx_mode = 0;        
      }
      else
      {
        rx_sync_count++;
        
        if((rx_sample == 1) && 
           (rx_sync_count >= 20) && 
           (rx_count > MinLongCount))
        {
          // We have seen at least 10 regular transitions
          // Lock sequence ends with 01
          // This is TX as HI,LO,LO,HI
          // We have seen a long low - we are now locked!
          rx_mode = 2;
          rx_manBits = 0;
          rx_numMB = 0;
          rx_curByte = 0;
          rx_startbit = true;
        }
        else if (rx_sync_count >= 32)
        {
          rx_mode = 0;
        }
        rx_count = 0;
      }
    }
  }
  else if (rx_mode == 2)
  {
    // Receive data
    if (transition)
    {
      if((rx_count < MinCount) ||
         (rx_count > MaxLongCount))
      {
        // Interference - give up
        rx_mode = 0;
      }
      else
      {
        AddManBit(&rx_manBits, &rx_numMB, &rx_curByte, rx_data, rx_last_sample);
        if(rx_count > MinLongCount)  //is this a double bit
        {
          AddManBit(&rx_manBits, &rx_numMB, &rx_curByte, rx_data, rx_last_sample);
        }
        
        if (rx_curByte >= rx_maxBytes)
        {
          rx_mode == 3;
        }
        
        rx_count = 0;
      }
    }
  }
  else if (rx_mode == 3)
  {
    // Got total message - do nothing
  }
  
  // Get ready for next loop
  rx_last_sample = rx_sample;
}

const int MSG_SIZE = 3;
const int MAX_NODE_ID = 31;
unsigned int msgNum[MAX_NODE_ID];
const int MAX_MSG_NUM = 31;

void setup() 
{   
 Serial.begin(9600);
  
 //ATMega328 timer 2 (http://www.atmel.com/dyn/resources/prod_documents/doc8161.pdf)
 TCCR2A = _BC(WGM21); // reset counter on match
 TCCR2B = _BV(CS22) | _BV(CS21); //counts every 16 usec with 16 Mhz clock
 OCR2A = 5; // interrupt every 5 counts
 TIMSK2 = _BV(OCIE2A);
 TCNT2 = 0;
 
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
  delay(1000);
  Serial.print("Sync: ");
  Serial.print(rx_sync_count);
  Serial.print(", Mode: ");
  Serial.print(rx_mode);
  Serial.print(", Max_Mode: ");
  Serial.print(rx_max_mode);  
  Serial.println();
  /*readMsg();
  Serial.print("Read data from node ");
  Serial.print(xoNodeID);
  Serial.print(": ");
  Serial.println(xoData);*/
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
        
    if (rx_mode == 3)
    {
      unsigned int data = (((int)rx_data[0]) << 8) | (int)rx_data[1];
      rx_mode = 0;
      
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

