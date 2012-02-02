#include <avr/interrupt.h>

// ATTiny85:
//             u
//   Reset (1)    (8) VCC
// P3 (A3) (2)    (7) P2 (A1)
// P4 (A2) (3)    (6) P1 (PWM)
//     GND (4)    (5) P0 (PWM)

#define TIMER_CLOCK_FREQ 8000000.0

void setup() 
{
  // We don't use pins 0-2 so set these to INPUT
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  
  // Pins 3, 4 are used for output
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  
  // Pin 3 is used as a controlled VCC while we are awake
  digitalWrite(3, HIGH);
  
  // Preset pin 4 to low
  digitalWrite(4, LOW);
  
  // Timer clock: 8MHz / 8 = 1MHz
  // 200 tics at 1MHz = 200uS/interrupt
  TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS11); 
  // Enable timer interrupts
  TIMSK |= _BV(OCIE1A);  
  OCR1A = 102;
}//end of setup

unsigned long count = 0;
int ledon = 0;

/*
8MHz => 1000000 / 8000000 = 0.125uS/tic
10Hz => 1000000 / 10 => 100000uS/tic
100000 / 0.125 = 800000 tics

8MHz/256 => 1000000 / (8000000 / 256) = 32uS/tic
10Hz => 1000000 / 10 => 100000uS/tic
100000 / 32 = 3125 tics

8MHz/8192 => 1000000 / (8000000 / 8192) = 976.5625uS/tic
10Hz => 1000000 / 10 => 100000uS/tic
100000 / 976.5625 = 102.4 tics

8MHz/256 => 1000000 / (8000000 / 256) = 32uS/tic
1000Hz => 1000000 / 1000 => 1000uS/tic
1000 / 32 = 31.25 tics

8MHz/128 => 1000000 / (8000000 / 128) = 16uS/tic
1000Hz => 1000000 / 1000 => 1000uS/tic
1000 / 16 = 62.5 tics

8MHz/64 => 1000000 / (8000000 / 128) = 8uS/tic
1000Hz => 1000000 / 1000 => 1000uS/tic
1000 / 8 = 125 tics

*/

SIGNAL(TIMER1_COMPA_vect)
{
  if (ledon == 0)
  {
    digitalWrite(4, HIGH);
    ledon = 1;
  }
  else
  {
    digitalWrite(4, LOW);
    ledon = 0;
  }
}

void loop() 
{
  // do nothing
}
