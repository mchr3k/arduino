/*
This code is based on the Atmel Corporation Manchester
Coding Basics Application Note.

http://www.atmel.com/dyn/resources/prod_documents/doc9164.pdf

Quotes from the application note:

"Manchester coding states that there will always be a transition of the message signal 
at the mid-point of the data bit frame. 
What occurs at the bit edges depends on the state of the previous bit frame and
does not always produce a transition. A logical “1” is defined as a mid-point transition
from low to high and a “0” is a mid-point transition from high to low.

We use Timing Based Manchester Decode.
In this approach we will capture the time between each transition coming from the demodulation
circuit."

Timer 2 is used with a ATMega328. Timer 1 is used for a ATtiny85.

This code gives a basic data rate as 1000 bits/s. In manchester encoding we send 1 0 for a data bit 0.
We send 0 1 for a data bit 1. This ensures an average over time of a fixed DC level in the TX/RX.
This is required by the ASK RF link system to ensure its correct operation.
The actual data rate is then 500 bits/s.
*/

#include "MANCHESTER.h"

MANCHESTERClass::MANCHESTERClass()  //constructor
{
 TxPin = TxDefault;
 pinMode(TxPin, OUTPUT);      // sets the digital pin 4 default as output 
 RxPin = RxDefault;  //sets the digital pin 4 default as input 
 pinMode(RxPin, INPUT);      // default digital pin as output
 TimeOut = TimeOutDefault; //default is to block

  #if defined( __AVR_ATtinyX5__ )
  #define TimerCount TCNT1   //ATtiny85 timer 1  
 #else
   //ATMega328 timer 2
   #define TimerCount TCNT2   //ATMega328 timer 2
   TCCR2A = 0x00;
   TCCR2B = 0x06; //counts every 16 usec with 16 Mhz clock
   TIMSK2 = 0x00;
 #endif   
}//end of constructor

/*
The 433.92 Mhz receivers have AGC, if no signal is present the gain will be set 
to its highest level.

In this condition it will switch high to low at random intervals due to input noise.
A CRO connected to the data line looks like 433.92 is full of transmissions.

Any ASK transmission method must first sent a capture signal of 101010........
When the receiver has adjusted its AGC to the required level for the transmisssion
the actual data transmission can occur.

We send 14 0's 1010...    It takes 1 to 3 10's for the receiver to adjust to 
the transmit level.

The receiver waits until we have at least 10 10's and then a start pulse 01.
The receiver is then operating correctly and we have locked onto the transmission.
*/
unsigned int MANCHESTERClass::Receive(void)
{
#if defined( __AVR_ATtinyX5__ )
  TCCR1 = 0x08;  //counts every 16 usec with 8Mhz clock
  TIMSK = 0;
#endif

unsigned long timeoutstart = millis();

for(;;)  //we keep looking at the receiver output for a transmission
{
unsigned char countlow = 0;
unsigned char counthigh = 0;

boolean locked = true;

while(digitalRead(RxPin) == 0);//wait until set low to high

for(int i = 0; i < 16; i++)  //check at least 32 pulses
{
TimerCount = 0;
while(digitalRead(RxPin) != 0)
counthigh = TimerCount;  //end of count is high

if((counthigh < LowCount) || (counthigh > HighCount))
{
locked = false;  
break; //this cant be a valid pulse
}//end of not valid

TimerCount = 0;
while(digitalRead(RxPin) == 0)
countlow = TimerCount;  //end of count is low

if((counthigh < LowCount) || (counthigh > HighCount))
{
locked = false;  
break; //this cant be a valid pulse
}//end of not valid

if((i > 9) && (countlow > LongCount))
break; //we have a capture lock and start pulse
}//end of get 10 capture pulses and start pulse

if(millis() - timeoutstart > TimeOut)
return 0;  //timed out so return to caller

unsigned long ManBits = 0; //the received manchester 32 bits
unsigned char NumMB = 0;  //the number of received manchester bits
boolean PreviousBit_One = true;  //the start bit is a one

if(locked)  //have we detected a capture pulse train
{
for(int i = 0; i < 17; i++) //check at least 32 bits
{
ManBits |= _BV(0);  //incorporate 1 in manbits
NumMB += 1; //count the bit
if(NumMB == 33)
break;  //we have the whole 32 manchester bits
ManBits = ManBits << 1; //and move it into store

TimerCount = 0;
while(digitalRead(RxPin) != 0)
counthigh = TimerCount;  //end of count is high

if(counthigh > LongCount)  //is this a double 1
{
ManBits |= _BV(0);  //incorporate 1 in manbits
NumMB += 1; //count the bit
if(NumMB == 33)
break;  //we have the whole 32 manchester bits
ManBits = ManBits << 1; //and move it into store  
}//end of we have a double 1

ManBits &= ~_BV(0);  //incorporate 0 in manbits
NumMB += 1; //count the bit
if(NumMB == 33)
break;  //we have the whole 32 manchester bits
ManBits = ManBits << 1; //and move it into store

TimerCount = 0;
while(digitalRead(RxPin) == 0)
countlow = TimerCount;  //end of count is low

if(countlow > LongCount)  //is this a double 0
{
ManBits &= ~_BV(0);  //incorporate 0 in manbits
NumMB += 1; //count the bit  
if(NumMB == 33)
break;  //we have the whole 32 manchester bits
ManBits = ManBits << 1; //and move it into store
}//end of we have a double 1
}//end of get at least 32 bits bits

 unsigned int data = 0; //the received 16 bits 
 for(int i = 0; i < 16; i++)
 {
 data = data << 1; //move the bit into data
 if(ManBits & _BV(0) == 1) //is the data bit a 1
 data |= _BV(0);  //store the one
 ManBits = ManBits >> 2; //get next data bit
 }//end of get the received data from manchester bits
 
return data; 
}//end of its locked
}//end of look until find data or timeout
}//end of receive data

void MANCHESTERClass::SetTimeOut(unsigned int timeout)
{
TimeOut = timeout;  //user sets timeout 
}//end of user sets timeout

void MANCHESTERClass::SetRxPin(char pin)
{
 RxPin = pin;
 pinMode(pin, INPUT);      // user sets the digital pin as input  
}//end of set transmit pin	

void MANCHESTERClass::SetTxPin(char pin)
{
  TxPin = pin;      // user sets the digital pin as output
  pinMode(TxPin, OUTPUT);      // sets the digital pin 4 default as output   
}//end of set transmit pin	


void MANCHESTERClass::Transmit(unsigned int data)
{
 for( int i = 0; i < 14; i++) //send capture pulses
 sendzero();  //end of capture pulses
 
 sendone();  //start data pulse
 
 unsigned int mask = 0x01; //mask to send bits 
 for( int i = 0; i < 16; i++)
 {
 if((data & mask) == 0)
 sendzero();
 else
 sendone();
 mask = mask << 1; //get next bit
 }//end of pulses
 
 sendzero();
 sendzero();  // 2 end pulses  
}//end of send the data	

inline void MANCHESTERClass::sendzero(void)
{
 digitalWrite(TxPin, HIGH);   
 delayMicroseconds(1000);        

 digitalWrite(TxPin, LOW);      
 delayMicroseconds(1000);        
}//end of send a zero

inline void MANCHESTERClass::sendone(void)
{
 digitalWrite(TxPin, LOW);      
 delayMicroseconds(1000);

 digitalWrite(TxPin, HIGH);   
 delayMicroseconds(1000);    
}//end of send one

MANCHESTERClass MANCHESTER;
