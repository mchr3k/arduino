#include <MANCHESTER.h>
#include <ATTinyWatchdog.h>
#include <avr/power.h>

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)

#define TxPin 4  //the digital pin to use to transmit data
const int NODE_ID = 1;

unsigned int Tdata = 0;  //the 16 bits to send

void setup() 
{
  // Prepare for random pauses
  randomSeed(analogRead(0));  
  // Prepare for manchester RF TX
  MANCHESTER.SetTxPin(TxPin);
  // Setup watchdog to notify us every 4 seconds
  ATTINYWATCHDOG.setup(8);
  // Turn off subsystems which we aren't using  
  power_timer0_disable();
  // timer1 used by MANCHESTER
  power_usi_disable();
  // ADC used for reading a sensor
  // ATTINYWATCHDOG turns off ADC before sleep and
  // restores it when we wake up
  
  // We don't use pins 0-2 so set these to INPUT
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  
  // Pins 3, 4 are used for output
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  
  // Pin 3 is used as a controlled VCC while we are awake
  digitalWrite(3, HIGH);
}//end of setup

void loop() 
{
  Tdata +=1;
  sendMsg(Tdata);    
  deepsleep();
}//end of loop

void deepsleep()
{
  // Turn our VCC pin off
  digitalWrite(3, LOW);
  // Set pins to input before sleep
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  // deep sleep for 2 * 4 seconds = 8 seconds
  ATTINYWATCHDOG.sleep(2);  
  // Set pins to output after sleep
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  // Pin 3 is used as a controlled VCC while we are awake
  digitalWrite(3, HIGH);
}

void livesleep(unsigned long time)
{
  // Turn our VCC pin off
  digitalWrite(3, LOW);
  // Set output pins to input
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  // delay for the requested time
  delay(time);  
  // Set pins to output after sleep
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  // Pin 3 is used as a controlled VCC while we are awake
  digitalWrite(3, HIGH);
}

unsigned int readingNum = 1;

void sendMsg(unsigned int data)
{
  readingNum++;
  if (readingNum >= 31) readingNum = 0;
  
  doSendMsg(data, readingNum);
  
  livesleep(500 + random(500));
  doSendMsg(data, readingNum);
  
  livesleep(500 + random(500));
  doSendMsg(data, readingNum);
}

void doSendMsg(unsigned int data, unsigned int msgNum)
{       
  // Send a message with the following format
  // 6 bits pre-amble
  // 5 bit node ID
  // 5 bit reading number
  // 16 bit data
  // 16 bit data (repeated)
  //
  // This is a total of 3x unsigned ints     
  unsigned int preamble = (0b010101 << 10);
  unsigned int nodeID = ((NODE_ID & 0b11111) << 5);
  unsigned int firstPacket = preamble | nodeID | (msgNum & 0b11111);
  
  MANCHESTER.Transmit(firstPacket);
  MANCHESTER.Transmit(data);
  MANCHESTER.Transmit(data);
}
