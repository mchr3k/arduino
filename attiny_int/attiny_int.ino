#define LedPin 2

void setup() 
{
  // Setup LedPin
  pinMode(LedPin, OUTPUT);
  
  // We will use Timer 2 on the ATmega328.
  // See section 17 of http://www.atmel.com/dyn/resources/prod_documents/doc8161.pdf
  // Section 17.11 contains details about the registers
  // which need to be setup.
  //
  // Table 17-8 in the datasheet describes how to set the
  // counter mode. To use CTC we must set _BV(WGM21)
  // in register TCCR2A.
  TCCR2A = _BV(WGM21);
  // Table 17-9 shows the available clock prescale values.
  // We will use the highest available: 1/1024. This
  // requires that we set 4 bits.
  // This gives a clock of 16MHz/1024 = 15625Hz
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
  // We want an interrupt once per second. This would
  // require a compare value of 15625. However OCR2A
  // is only an 8 bit register which can hold a max value
  // of 255.
  //
  // The highest factor of 15625 lower than 255 is 125
  // so we will use that. 15625 = 125 * 125 so settings
  // OCR2A to 125 will generate 125 interrupts per second.
  // We will need to manually count how many times
  // we have been interrupted and flash our LED every time
  // we are interrupted for the 125th time.
  OCR2A = 125;
  // Finally we need to enable our interrupt.
  TIMSK2 = _BV(OCIE2A);
}

unsigned long count = 0;
int ledon = 0;
SIGNAL(TIMER2_COMPA_vect)
{
  // We expect 125 interrupts a second
  count++;
  if (count > 125)
  {
    count = 0;
    if (ledon == 0)
    {
      digitalWrite(LedPin, HIGH);
      ledon = 1;
    }
    else
    {
      digitalWrite(LedPin, LOW);
      ledon = 0;
    }
  }
}

void loop() 
{
  // do nothing
}
