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

#define RX_MODE_PRE  0
#define RX_MODE_SYNC 1
#define RX_MODE_DATA 2
#define RX_MODE_MSG  3
#define RX_MODE_IDLE 4

static int rx_sample = 0;
static int rx_last_sample = 0;
static uint8_t rx_count = 0;
static uint8_t rx_sync_count = 0;
static uint8_t rx_mode = RX_MODE_IDLE;

unsigned int rx_manBits = 0; //the received manchester 32 bits
unsigned char rx_numMB = 0;  //the number of received manchester bits
unsigned char rx_curByte = 0;

unsigned char rx_maxBytes = 2;
unsigned char rx_default_data[2];
unsigned char* rx_data = rx_default_data;

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

void rx_setup()
{  
  //ATMega328 timer 2 (http://www.atmel.com/dyn/resources/prod_documents/doc8161.pdf)
  TCCR2A = _BV(WGM21); // reset counter on match
  TCCR2B = _BV(CS22) | _BV(CS21); //counts every 16 usec with 16 Mhz clock
  OCR2A = 4; // interrupt every 5 counts (0->4)
  TIMSK2 = _BV(OCIE2A);
  TCNT2 = 0;
}

void rx_begin()
{
  rx_maxBytes = 2;
  rx_data = rx_default_data;
  rx_mode = RX_MODE_PRE;
}

void rx_begin(unsigned char maxBytes, unsigned char *data)
{
  rx_maxBytes = maxBytes;
  rx_data = data;
  rx_mode = RX_MODE_PRE;  
}

void rx_pause()
{
  rx_mode = RX_MODE_IDLE;
}

boolean rx_gotmsg()
{
  return (rx_mode == RX_MODE_MSG);
}

unsigned int rx_getmsg()
{
  return (((int)rx_data[0]) << 8) | (int)rx_data[1];
}

void rx_getmsg(unsigned char *rcvdBytes, unsigned char **data)
{
  *rcvdBytes = rx_curByte;
  *data = rx_data;
}

ISR(TIMER2_COMPA_vect)
{
  if (rx_mode < 3)
  {
    // Increment counter  
    rx_count += 5;
    
    // Check for value change
    rx_sample = digitalRead(RxPin);
    boolean transition = (rx_sample != rx_last_sample);
  
    if (rx_mode == RX_MODE_PRE)
    {
      // Wait for first transition to HIGH
      if (transition && (rx_sample == 1))
      {
        rx_count = 0;
        rx_sync_count = 0;
        rx_mode = RX_MODE_SYNC;
      }
    }
    else if (rx_mode == RX_MODE_SYNC)
    {
      // Initial sync block
      if (transition)
      {
        if(((rx_sync_count < 20) || (rx_last_sample == 1)) && 
           ((rx_count < MinCount) || (rx_count > MaxCount)))
        {
          // First 20 bits and all 1 bits are expected to be regular
          // Transition was too slow/fast
          rx_mode = RX_MODE_PRE;
        }
        else if((rx_last_sample == 0) &&
                ((rx_count < MinCount) || (rx_count > MaxLongCount)))
        {
          // 0 bits after the 20th bit are allowed to be a double bit
          // Transition was too slow/fast
          rx_mode = RX_MODE_PRE;
        }
        else
        {
          rx_sync_count++;
          
          if((rx_last_sample == 0) && 
             (rx_sync_count >= 20) && 
             (rx_count > MinLongCount))
          {
            // We have seen at least 10 regular transitions
            // Lock sequence ends with unencoded bits 01
            // This is encoded and TX as HI,LO,LO,HI
            // We have seen a long low - we are now locked!
            rx_mode = RX_MODE_DATA;
            rx_manBits = 0;
            rx_numMB = 0;
            rx_curByte = 0;
          }
          else if (rx_sync_count >= 32)
          {
            rx_mode = RX_MODE_PRE;
          }
          rx_count = 0;
        }
      }
    }
    else if (rx_mode == RX_MODE_DATA)
    {
      // Receive data
      if (transition)
      {
        if((rx_count < MinCount) ||
           (rx_count > MaxLongCount))
        {
          // Interference - give up
          rx_mode = RX_MODE_PRE;
        }
        else
        {
          if(rx_count > MinLongCount)  // was the previous bit a double bit?
          {
            AddManBit(&rx_manBits, &rx_numMB, &rx_curByte, rx_data, rx_last_sample);
          }
          if ((rx_sample == 1) &&
              (rx_curByte >= rx_maxBytes))
          {
            rx_mode = RX_MODE_MSG;
          }
          else
          {
            // Add the current bit
            AddManBit(&rx_manBits, &rx_numMB, &rx_curByte, rx_data, rx_sample);          
            rx_count = 0;
          }        
        }
      }
    }
    
    // Get ready for next loop
    rx_last_sample = rx_sample;
  }
}

const int MSG_SIZE = 3;
const int MAX_NODE_ID = 31;
unsigned int msgNum[MAX_NODE_ID];
const int MAX_MSG_NUM = 31;

void setup()
{   
 Serial.begin(9600);
 
 rx_setup();
 rx_begin();
 
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
    if (rx_gotmsg())
    {
      offset++;
      if (offset >= MSG_SIZE) offset = 0;
      
      unsigned int data = rx_getmsg();
      rx_begin();
      
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

