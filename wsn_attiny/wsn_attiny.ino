#include <MANCHESTER.h>
#include <ATTinyWatchdog.h>
#include <avr/power.h>
#include <avr/interrupt.h>

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)
//
// Temp calibration formula:
// V_calib = V_base * ((T_ref + 50) / (T_base + 50))

#define TmpPin 1 // Analog pin 1 - digital pin 2
#define TmpDPin 2
#define TmpPwr 1
#define TxPin 4
#define TxPwr 3

const int NODE_ID = 1;

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
  
  // Select the 1.1V internal ref voltage
  analogReference(INTERNAL);
  
  // We don't use pin 0 so set this to INPUT
  pinMode(0, INPUT);  
}//end of setup

unsigned int testdata = 1;

void loop() 
{
  waketmp();
  delay(10);
  unsigned int data = getTemp();
  sleeptmp();
  
  waketx();
  delay(10);
  sendMsg(data);
  sleeptx();
  
  deepsleep(70); // 4 minutes, 40 seconds
}

unsigned int getTemp()
{
  return (unsigned int)analogRead(TmpPin);
}

void sleeptx()
{
  // Pins 3, 4 are used for Tx
  pinMode(TxPin, INPUT);
  pinMode(TxPwr, INPUT);
}

void sleeptmp()
{
  // Pins 1, 2 are used for TMP
  pinMode(TmpDPin, INPUT);
  pinMode(TmpPwr, INPUT);  
}

void waketx()
{
  // Pins 3, 4 are used for Tx
  pinMode(TxPin, OUTPUT);
  pinMode(TxPwr, OUTPUT);
  
  // Power up the TX and TMP
  digitalWrite(TxPwr, HIGH);
}

void waketmp()
{
  // Pins 1, 2 are used for TMP
  pinMode(TmpDPin, INPUT);
  pinMode(TmpPwr, OUTPUT);
  
  // Power up the TX and TMP
  digitalWrite(TmpPwr, HIGH);
}

void deepsleep(unsigned int multiple)
{  
  sleeptx();
  sleeptmp();
  // deep sleep for multiple * 4 seconds
  ATTINYWATCHDOG.sleep(multiple);
  waketx();
  waketmp();
}

unsigned int readingNum = 1;

void sendMsg(unsigned int data)
{
  readingNum++;
  if (readingNum >= 31) readingNum = 0;
  
  doSendMsg(data, readingNum);
  
  deepsleep(random(1,2));
  doSendMsg(data, readingNum);
  
  deepsleep(random(1,2));
  doSendMsg(data, readingNum);
}

void doSendMsg(unsigned int xiData, unsigned char xiMsgNum)
{       
  // Send a message with the following format
  // 5 bit node ID
  // 5 bit reading number
  // 6 bit unused
  // 16 bit data
  // 8 bit checksum
  
  // This is a total of 5 unsigned chars
  unsigned char databuf[5];
  unsigned char nodeID = (NODE_ID & 0b11111);
  unsigned char msgNum = (xiMsgNum  & 0b11111);  
  
  databuf[0] = (nodeID << 3) | (msgNum >> 2);
  databuf[1] = ((msgNum & 0b00000011) << 6);
  databuf[2] = ((0xFF00 & xiData) >> 8);
  databuf[3] = (0x00FF & xiData);
  databuf[4] = databuf[0] | databuf[1] | databuf[2] | databuf[3];
  
  MANCHESTER.TransmitBytes(5, databuf);
}
