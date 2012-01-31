
// ArduinoISP version 04m2

/*
  Copyright (c) 2008-2011 Randall Bohn
  Copyright (c) 2009 David A. Mellis
  Copyright (c) 2011-2012 Rowdy Dog Software
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 
    
  * Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 
    
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  POSSIBILITY OF SUCH DAMAGE.

  http://www.opensource.org/licenses/bsd-license.php
*/

// this sketch turns the Arduino into a AVRISP
// using the following pins:
// 10: slave reset
// 11: MOSI
// 12: MISO
// 13: SCK

// Put an LED (with resistor) on the following pins:
// 9: Heartbeat - shows the programmer is running
// 8: Error - Lights up if something goes wrong (use red if that makes sense)
// 7: Programming - In communication with the slave
//
// October 2010 by Randall Bohn
// - Write to EEPROM > 256 bytes
// - Better use of LEDs:
// -- Flash LED_PMODE on each flash commit
// -- Flash LED_PMODE while writing EEPROM (both give visual feedback of writing progress)
// - Light LED_ERR whenever we hit a STK_NOSYNC. Turn it off when back in sync.
//
// October 2009 by David A. Mellis
// - Added support for the read signature command
// 
// February 2009 by Randall Bohn
// - Added support for writing to EEPROM (what took so long?)
// Windows users should consider WinAVR's avrdude instead of the
// avrdude included with Arduino software.
//
// January 2008 by Randall Bohn
// - Thanks to Amplificar for helping me with the STK500 protocol
// - The AVRISP/STK500 (mk I) protocol is used in the arduino bootloader
// - The SPI functions herein were developed for the AVR910_ARD programmer 
// - More information at http://code.google.com/p/mega-isp

#include <pins_arduino.h>

#if ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif


/*----------------------------------------------------------------------------*/

//#define PROGRAMMER_BAUD_RATE              250000
#define PROGRAMMER_BAUD_RATE              19200

#define PROGRAMMER_USE_ONE_LED            0
#define PROGRAMMER_USE_OLD_LED_LAYOUT     1

#define PROGRAMMER_USE_FAST_SPI_CLOCK     0
#define PROGRAMMER_USE_NORMAL_SPI_CLOCK   1
#define PROGRAMMER_USE_SLOW_SPI_CLOCK     0

#define RELAY_ENABLED                     0
#define RELAY_SAY_HELLO                   1
//#define RELAY_BAUD_RATE                   38400
#define RELAY_BAUD_RATE                   9600
#define RELAY_TICK_PIN                    8

#define EXTRA_OUTPUT_TUNING_CLOCK         0

#define EXTRA_OUTPUT_RECOVERY_CLOCK       0

#define EXTRA_OUTPUT_TUNING_PULSE         0
#define EXTRA_USE_LONG_TUNING_PULSE       0

/*----------------------------------------------------------------------------*/

#if EXTRA_OUTPUT_TUNING_CLOCK + EXTRA_OUTPUT_TUNING_PULSE > 1
#error EXTRA_OUTPUT_TUNING_CLOCK and EXTRA_OUTPUT_TUNING_PULSE are mutually exclusive.  Enable only one.
#endif

#if PROGRAMMER_USE_FAST_SPI_CLOCK + PROGRAMMER_USE_NORMAL_SPI_CLOCK + PROGRAMMER_USE_SLOW_SPI_CLOCK != 1
#error PROGRAMMER_USE_FAST_SPI_CLOCK, PROGRAMMER_USE_NORMAL_SPI_CLOCK, and PROGRAMMER_USE_SLOW_SPI_CLOCK are mutually exclusive.  Enable only one.
#endif

/*----------------------------------------------------------------------------*/

#if PROGRAMMER_USE_ONE_LED

class AispLED
{
public:

  void begin( uint8_t pin )
  {
    _pin = pin;
    pinMode( _pin, OUTPUT );
    _state = sLampTest;
    _next = sHeartbeat;
    _mode = sHeartbeat;
    _previousTick = millis();
    _heartbeat = +1;
  }

  void error( void )
  {
    if ( ! ( (_state >= sError0) && (_state <= sErrorN) ) )
    {
      _state = sError;
      update();
    }
  }

  void flash( void )
  {
//rmv    if ( ! ( (_state >= sError0) && (_state <= sErrorN) ) )
    {
      if ( (_state >= sFlash0) && (_state <= sFlashN) )
      {
        _next = sFlash2;
      }
      else
      {
        _state = sFlash;
        update();
      }
    }
  }

/* rmv
  void heartbeat( void )
  {
    _state = sHeartbeat;
    update();
  }
*/

  typedef enum 
  { 
    mProgrammer, mRelay
  } 
  mode_t;

  void setMode( mode_t mode )
  {
    if ( mode == mRelay )
    {
      _mode = sSilent;
    }
    else
    {
      _mode = sHeartbeat;
    }
  }

  void update( void )
  {
    state_t CurrentState;
    tick_t currentTick;
    tick_t delta;
    
    currentTick = millis();

    do
    {
      CurrentState = _state;
    
      switch ( _state )
      {
        case sFlash:
          analogWrite( _pin, 255 );
          _previousTick = currentTick;
          _state = sFlash1;
          break;
  
        case sFlash1:
          if ( currentTick - _previousTick >= 50 )
          {
            if ( _next != sFlash2 )
            {
              _fade = 128;
              _heartbeat = -1;
              analogWrite( _pin, _fade );
            }
            else
            {
              analogWrite( _pin, 0 );
            }
            _previousTick = currentTick;
            _state = _next;
            _next = _mode;
          }
          break;
  
        case sFlash2:
          if ( currentTick - _previousTick >= 50 )
          {
            analogWrite( _pin, 255 );
            _previousTick = currentTick;
            _state = sFlash1;
          }
          break;
  
  /* rmv
        case sFlash:
          _fade = 255;
          analogWrite( _pin, _fade );
          _previousTick = currentTick;
          _state = sFlash1;
          break;
    
        case sFlash1:
          if ( currentTick - _previousTick >= 8 )
          {
            _fade = (31 * _fade) / 32;
    //        _fade = (63 * _fade) / 64;
            analogWrite( _pin, _fade );
            if ( _fade <= 192 )
            {
              _state = sHeartbeat;
              _heartbeat = -1;
            }
            _previousTick = currentTick;
          }
          break;
  */
  
        case sError:
          analogWrite( _pin, 255 );
          _previousTick = currentTick;
          _state = sError1;
          break;
          
        case sError1:
          if ( currentTick - _previousTick >= 900 )
          {
            analogWrite( _pin, 0 );
            _previousTick = currentTick;
            _state = sError2;
          }
          break;
  
        case sError2:
          if ( currentTick - _previousTick >= 100 )
          {
            analogWrite( _pin, 255 );
            _previousTick = currentTick;
            _state = sError1;
          }
          break;
  
        case sHeartbeat:
          _previousTick = currentTick;
          _state = sHeartbeat1;
          break;
  
        case sHeartbeat1:
          if ( currentTick - _previousTick >= 48 /*16*/ )
          {
            if ( _fade >= 48 /*64*/ )
            {
              _heartbeat = -1;
            }
            else if ( _fade <= 8 )
            {
              _heartbeat = +1;
              
              if ( _mode == sSilent )
              {
                _state = sSilent;
              }
            }
            _fade = _fade + _heartbeat;
            analogWrite( _pin, _fade );
            _previousTick = currentTick;
          }
          break;
          
        case sSilent:
          analogWrite( _pin, 0 );
          _state = sSilent1;
          break;
          
        case sSilent1:
            if ( _mode == sHeartbeat )
            {
              _state = sHeartbeat;
            }
          break;

        case sLampTest:
          _fade = 0;
          analogWrite( _pin, 255 );
          _previousTick = currentTick;
          _state = sLampTest1;
          break;

        case sLampTest1:
          delta = currentTick - _previousTick;
          if ( delta >= 50 )
          {
            ++_fade;
            if ( _fade <= 4 )
            {
              analogWrite( _pin, 255 );
              _previousTick = currentTick;
            }
            else
            {
              _state = _mode;
            }
          }
          else
            analogWrite( _pin, 255 - (5*delta) );
          break;
      }
    }
    while ( CurrentState != _state );
  }

private:

  typedef enum 
  { 
    sFlash0, sFlash, sFlash1, sFlash2, sFlashN,
    sError0, sError, sError1, sError2, sErrorN,
    sHeartbeat0, sHeartbeat, sHeartbeat1, sHeartbeatN,
    sSilent0, sSilent, sSilent1, sSilentN,
    sLampTest, sLampTest1,
    sFini 
  } 
  state_t;
  
  typedef unsigned short tick_t;
  
  uint8_t _pin;
  tick_t _previousTick;
  state_t _state;
  state_t _next;
  state_t _mode;
  int16_t _fade;
  int16_t _heartbeat;
};

AispLED TheLED;

#endif

/*----------------------------------------------------------------------------*/

#if RELAY_ENABLED

  #if ARDUINO >= 100
    #include <SoftwareSerial.h>
    SoftwareSerial Relay( 12, 14 );
  #else
//    #include <NewSoftSerial.h>
//    NewSoftSerial Relay( 12, 14 );
  #endif
  
  bool HoldInResetAfterProgramming;
  bool HeldInReset;
  bool PreviousTick;

#endif

/*----------------------------------------------------------------------------*/

#define RESET SS

#if PROGRAMMER_USE_ONE_LED
  #define LED_PIN 5
#else
  #if PROGRAMMER_USE_OLD_LED_LAYOUT
    #define LED_HB 9
    #define LED_ERR 8
    #define LED_PMODE 7
    #define PROG_FLICKER true
  #else
    #define LED_HB 5
    #define LED_ERR 6
    #define LED_PMODE 7
    #define PROG_FLICKER true
  #endif
#endif

#define HWVER 2
#define SWMAJ 1
#define SWMIN 18

// STK Definitions
#define STK_OK 0x10
#define STK_FAILED 0x11
#define STK_UNKNOWN 0x12
#define STK_INSYNC 0x14
#define STK_NOSYNC 0x15
#define CRC_EOP 0x20 //ok it is a space...

/*----------------------------------------------------------------------------*/

#if ! PROGRAMMER_USE_ONE_LED
void pulse(int pin, int times);
#endif

/*----------------------------------------------------------------------------*/

void setup() 
{
  Serial.begin( PROGRAMMER_BAUD_RATE );
  
  #if PROGRAMMER_USE_ONE_LED
    TheLED.begin( LED_PIN );
  #else
    pinMode( LED_PMODE, OUTPUT );
    pulse( LED_PMODE, 2 );
    pinMode( LED_ERR, OUTPUT );
    pulse( LED_ERR, 2 );
    pinMode( LED_HB, OUTPUT );
    pulse( LED_HB, 2 );
  #endif

  #if EXTRA_OUTPUT_TUNING_CLOCK
    start_tuning_clock();
  #endif

  #if EXTRA_OUTPUT_TUNING_PULSE
    start_tuning_pulse();
  #endif

  #if EXTRA_OUTPUT_RECOVERY_CLOCK
    start_recovery_clock();
  #endif

  #if RELAY_ENABLED
    pinMode( RELAY_TICK_PIN, INPUT );
    digitalWrite( RELAY_TICK_PIN, HIGH );
  #endif
}

/*----------------------------------------------------------------------------*/

int pmode=0;
// address for reading and writing, set by 'U' command
int here;
uint8_t buff[256]; // global block storage

#define beget16(addr) (*addr * 256 + *(addr+1) )

typedef struct param 
{
  uint8_t devicecode;
  uint8_t revision;
  uint8_t progtype;
  uint8_t parmode;  
  uint8_t polling;
  uint8_t selftimed;
  uint8_t lockbytes;
  uint8_t fusebytes;
  uint16_t flashpoll;
  uint16_t eeprompoll;
  uint16_t pagesize;
  uint16_t eepromsize;
  uint32_t flashsize;
} 
parameter;

parameter param;

/*----------------------------------------------------------------------------*/

static unsigned error_count;
static uint8_t first_mark;
static unsigned command_count;

static void set_error( uint8_t _mark, uint8_t _extra )
{
  ++error_count;
}

/*----------------------------------------------------------------------------*/

#if PROGRAMMER_USE_ONE_LED
#else
// this provides a heartbeat on pin 9, so you can tell the software is running.
uint8_t hbval=128;
int8_t hbdelta=8;
unsigned long hbprev;
void heartbeat() {
  unsigned long current;
  current = millis();
  if ( current - hbprev >= 40 )
  {
    if (hbval > 192) hbdelta = -hbdelta;
    if (hbval < 32) hbdelta = -hbdelta;
    hbval += hbdelta;
    analogWrite(LED_HB, hbval);
    hbprev = current;
  }
//rmv  delay(40);
}
#endif
  
/*----------------------------------------------------------------------------*/

#if RELAY_ENABLED
static void serial_relay_output_stuff( void )
{
  Serial.println();
}
#endif

/*----------------------------------------------------------------------------*/

#if RELAY_ENABLED
void do_serial_relay( void )
{
  bool RelayActive;
  bool SomethingRelayed;
  bool ThisTick;
  
  Relay.begin( RELAY_BAUD_RATE );

  #if PROGRAMMER_USE_ONE_LED
    TheLED.setMode( AispLED::mRelay );
  #endif
  
  #if RELAY_SAY_HELLO
    Serial.println( "Serial Relay starting..." );
  #endif

  if ( HeldInReset )
  {
    pinMode( SCK, INPUT );
    pinMode( RESET, INPUT );
    #if RELAY_SAY_HELLO
      Serial.println( "Processor released from reset." );
    #endif
    HeldInReset = false;
  }
  
  RelayActive = true;
  SomethingRelayed = false;

//rmv  while( ! Serial.available() )
  while ( RelayActive )
  {
    while ( Relay.available() )
    {
      Serial.write( Relay.read() );
      SomethingRelayed = true;
    }

    #if PROGRAMMER_USE_ONE_LED
      if ( SomethingRelayed )
      {
        TheLED.flash();
        SomethingRelayed = false;
      }
    #endif
    
    ThisTick = digitalRead( RELAY_TICK_PIN );
    if ( ThisTick != PreviousTick )
    {
      PreviousTick = ThisTick;

      Serial.write( '\t' );
      Serial.print( ThisTick, DEC );
      Serial.write( '\t' );
      Serial.print( millis(), DEC );
      Serial.println();
    }

    if ( Serial.available() )
    {
      unsigned long Start;

      char ch = Serial.read();
      
      switch ( ch )
      {
        case '@':
          HoldInResetAfterProgramming = ! HoldInResetAfterProgramming;
          #if RELAY_SAY_HELLO
            Serial.print( "Processor " );
            if ( HoldInResetAfterProgramming )
            {
              Serial.print( "WILL" );
            }
            else
            {
              Serial.print( "will NOT" );
            }
            Serial.println( " be held in reset after programming" );
          #endif
          break;

        case '#':
          pinMode( RESET, OUTPUT );
/*rmv
          // The following should not be necessary.  It is kept because that's how it is done in start_pmode.
          digitalWrite( RESET, LOW );
*/
          Start = millis();
          #if RELAY_SAY_HELLO
            Serial.println( "Resetting target..." );
          #endif
          while ( millis() - Start < 50 );
          pinMode( RESET, INPUT );
          break;

        case '?':
          serial_relay_output_stuff();
          break;

        default:
          RelayActive = false;
          break;
      }
    }
    
    #if PROGRAMMER_USE_ONE_LED
      TheLED.update();
    #else
      heartbeat();
    #endif
  }

  while( Serial.available() )
    Serial.read();

  Relay.end();

  #if PROGRAMMER_USE_ONE_LED
    TheLED.setMode( AispLED::mProgrammer );
  #endif

  #if RELAY_SAY_HELLO
    Serial.println( "Serial Relay stopped" );
  #endif
}
#endif

/*----------------------------------------------------------------------------*/

#if EXTRA_OUTPUT_TUNING_CLOCK

static void start_tuning_clock( void )
{
  // Generate a 1.0 MHz clock on OC1A (PC6, digital pin 9)
  // Using a 1.0 MHz clock requires that the target run faster than 2.5 MHz so that the clock can reliably drive timer 0.  (F_CPU / 2.5)
  // The target decides what to do with the clock.
  
  // Turn the timer off while changes are made
  TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (0 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);

  // Configure the Compare Match Output Mode and the Waveform Generation Mode
  // COM1A1 COM1A0  = 0 1 = Toggle OC1A/OC1B on Compare Match.
  // COM1B1 COM1B0  = 0 0 = Normal port operation, OC1B disconnected.
  // WGM13 WGM12 WGM11 WGM10 = 0 1 0 0 = CTC OCR1A Immediate MAX
  TCCR1A =
      (0 << COM1A1) | (1 << COM1A0)
        |
      (0 << COM1B1) | (0 << COM1B0)
        |
      (0 << WGM11) | (0 << WGM10);

  TCCR1B =
      TCCR1B
        |
      (0 << WGM13) | (1 << WGM12);

  // No interrupts
  TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (0 << OCIE1A) | (0 << TOIE1);
  TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);

  // Ensure the first pulse is correct (fix? Should this be set to TOP on the Teensy?)
  TCNT1 = 0;

  // Frequency = F_CPU / (2 * Prescaler * (OCR + 1))
  // Frequency = 16000000 / (2 * 1 * (7 + 1))
  // Frequency = 1.0 MHz
  OCR1A = 7;

  // Enable the output driver
  DDRB |= (1 << DDB1);

  // Start the timer
  // CS12 CS11 CS10 = 0 0 1 = clkI/O/1 (No prescaling)
  TCCR1B =
      TCCR1B
        |
      ((0 << CS12) | (0 << CS11) | (1 << CS10));
}

static void stop_tuning_clock( void )
{
  // Stop the timer
  // CS12 CS11 CS10 = 0 0 0 = No clock source (Timer/Counter stopped).
  TCCR1B =
      TCCR1B
        &
      ~ ((0 << CS12) | (0 << CS11) | (1 << CS10));

  // Disable the output driver
  DDRB &= ~ (1 << DDB1);
}

#endif

/*----------------------------------------------------------------------------*/

#if EXTRA_OUTPUT_TUNING_PULSE
static void start_tuning_pulse( void )
{
  // Generate a 2.000 millisecond pulse on OC1A (PC6, digital pin 9)
  // If the target processor running at 8 MHz is perfectly tuned, TimeOnePulse returns 3200 counts (*5 = 16000 clocks) from a 2.000 millsecond pulse

  // Or
  // Generate a 16.000 millisecond pulse on OC1A (PC6, digital pin 9)
  // If the target processor running at 1 MHz is perfectly tuned, TimeOnePulse returns 3200 counts (*5 = 16000 clocks) from a 16.000 millsecond pulse

  // Turn the timer off while changes are made
  TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (0 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);

  // Configure the Compare Match Output Mode and the Waveform Generation Mode
  // COM1A1 COM1A0  = 1 0 = Clear OC1A on Compare Match, set OC1A at BOTTOM (non-inverting mode)
  // COM1B1 COM1B0  = 0 0 = Normal port operation, OC1B disconnected.
  // WGM13 WGM12 WGM11 WGM10 = 0 1 0 1 = Fast PWM, 8-bit 0x00FF BOTTOM TOP
  TCCR1A =
      (1 << COM1A1) | (0 << COM1A0)
        |
      (0 << COM1B1) | (0 << COM1B0)
        |
      (0 << WGM11) | (1 << WGM10);

  TCCR1B =
      TCCR1B
        |
      (0 << WGM13) | (1 << WGM12);

  // No interrupts
  TIMSK1 = (0 << ICIE1) | (0 << OCIE1B) | (0 << OCIE1A) | (0 << TOIE1);
  TIFR1 = (0 << ICF1) | (0 << OCF1B) | (0 << OCF1A) | (0 << TOV1);

  // Ensure the first pulse is the correct width (fix? Should this be set to TOP on the Teensy?)
  TCNT1 = 0;

  #if EXTRA_USE_LONG_TUNING_PULSE
    // (Prescaler / F_CPU) * (OCR + 1)
    // (1024 / 16000000) * (249 + 1)
    // 16 milliseconds
    OCR1A = 249;
  #else
    // (Prescaler / F_CPU) * (OCR + 1)
    // (256 / 16000000) * (124 + 1)
    // 2 milliseconds
    OCR1A = 124;
  #endif

  // Enable the output driver
//rmv  DDRB |= (1 << PB1);
  DDRB |= (1 << DDB1);

  // Start the timer
  #if EXTRA_USE_LONG_TUNING_PULSE
    // CS12 CS11 CS10 = 1 0 1 = clkI/O/1024 (From prescaler)
    TCCR1B =
        TCCR1B
          |
        ((1 << CS12) | (0 << CS11) | (1 << CS10));
  #else
    // CS12 CS11 CS10 = 1 0 0 = clkI/O/256 (From prescaler)
    TCCR1B =
        TCCR1B
          |
        ((1 << CS12) | (0 << CS11) | (0 << CS10));
  #endif
}
#endif

/*----------------------------------------------------------------------------*/

#if EXTRA_OUTPUT_RECOVERY_CLOCK
void start_recovery_clock( void )
{
  // Generate a 1 MHz clock on OC2B (PD3, digital pin 3)
  // The clock can be used to recover a processor that does not have an external crystal with the fuses set to use an external crystal

  // Turn the timer off while changes are made
  TCCR2B =
      (0 << FOC2A) | (0 << FOC2B) | (0 << WGM22) | (0 << CS22) | (0 << CS21) | (0 << CS20);

  // Configure the Compare Match Output Mode and the Waveform Generation Mode
  // COM2A1 COM2A0 = 0 0 = Normal port operation, OC0A disconnected.
  // COM2B1 COM2B0 = 0 1 = Toggle OC2B on Compare Match
  // WGM22 WGM21 WGM20 = 0 1 0 = CTC OCRA Immediate MAX
  TCCR2A =
      (0 << COM2A1) | (0 << COM2A0)
      |
      (0 << COM2B1) | (1 << COM2B0)
      |
      (1 << WGM21) | (0 << WGM20);

  TCCR2B =
      TCCR2B
        |
      (0 << WGM22);

  // No interrupts
  TIMSK2 =
      (0 << OCIE2B) | (0 << OCIE2A) | (0 << TOIE2);
  TIFR2 =
      (0 << OCF2B) | (0 << OCF2A) | (0 << TOV2);

  // Ensure the first pulse is correct (fix? Should this be set to TOP on the Teensy?)
  TCNT2 = 0;

  // F_CPU / (2 * Prescaler * (1 + OCR))
  // 16000000 / (2 * 1 * (1 + 7))
  // 1 MHz
  OCR2A = 7;

  // Enable the output driver
//rmv  DDRD |= (1 << PD3);
  DDRD |= (1 << DDD3);

  // Start the timer
  // CS22 CS21 CS20 = 0 0 1 = clkT2S/(No prescaling)
  TCCR2B =
      TCCR2B
        |
      ((0 << CS12) | (0 << CS11) | (1 << CS10));
}
#endif

/*----------------------------------------------------------------------------*/

uint8_t getch() {
  while(!Serial.available());
  return Serial.read();
}

/*----------------------------------------------------------------------------*/

void fill(int n) {
  for (int x = 0; x < n; x++) {
    buff[x] = getch();
  }
}

/*----------------------------------------------------------------------------*/

#if ! PROGRAMMER_USE_ONE_LED
#define PTIME 30
void pulse(int pin, int times) {
  do {
    digitalWrite(pin, HIGH);
    delay(PTIME);
    digitalWrite(pin, LOW);
    delay(PTIME);
  } 
  while (times--);
}
#endif

/*----------------------------------------------------------------------------*/

#if ! PROGRAMMER_USE_ONE_LED
void prog_lamp(int state) 
{
  if (PROG_FLICKER)
    digitalWrite(LED_PMODE, state);
}
#endif

/*----------------------------------------------------------------------------*/

#if ! PROGRAMMER_USE_SLOW_SPI_CLOCK

void spi_init() 
{
  uint8_t x;

//SPCR = 0x53;

#if PROGRAMMER_USE_NORMAL_SPI_CLOCK
  // SPE: SPI Enable
  // MSTR: Master/Slave Select
  // SPI2X SPR1 SPR0 = 0 1 0 = SCK Frequency is fosc/64 = 250 K
  // 250 K * 2 * 2 = 1 M
  SPCR = (0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) |  (0 << CPOL) | (0 << CPHA) | (1 << SPR1) | (0 << SPR0);
#endif

#if PROGRAMMER_USE_FAST_SPI_CLOCK
  // SPE: SPI Enable
  // MSTR: Master/Slave Select
  // SPI2X SPR1 SPR0 = 1 0 1 = SCK Frequency is fosc/8 = 2 M
  // 2 M * 2 * 2 = 8 M
  SPCR = (0 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR) |  (0 << CPOL) | (0 << CPHA) | (0 << SPR1) | (1 << SPR0);
  SPSR = SPSR | (1 << SPI2X);
#endif

  x=SPSR;
  x=SPDR;
}

#else
void spi_init() 
{
}
#endif

/*----------------------------------------------------------------------------*/

#if ! PROGRAMMER_USE_SLOW_SPI_CLOCK

void spi_wait() {
  do {
  } 
  while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_send(uint8_t b) {
  uint8_t reply;
  SPDR=b;
  spi_wait();
  reply = SPDR;
  return reply;
}

#else

uint8_t spi_send(uint8_t b) 
{
  uint8_t rv;
  
  rv = 0;

  for ( char i=7; i >= 0; --i )
  {
    rv = rv << 1;

    if ( b & 0x80 )
    {
      digitalWrite( MOSI, HIGH );
    }
    else
    {
      digitalWrite( MOSI, LOW );
    }

    // (1 / ((128000 / 2) / 2)) * 1000 * 1000 = 31.25
    digitalWrite( SCK, HIGH );
    delayMicroseconds( 32 );
    
    if ( digitalRead( MISO ) )
    {
      rv = rv | 0x01;
    }

    digitalWrite( SCK, LOW );
    delayMicroseconds( 32 );

    b = b << 1;
  }
  
  return( rv );
}

#endif


/*----------------------------------------------------------------------------*/

uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint8_t n;
  spi_send(a); 
  n=spi_send(b);
  //if (n != a) error_count = -1;
  n=spi_send(c);
  return spi_send(d);
}

/*----------------------------------------------------------------------------*/

void empty_reply() {
  if (CRC_EOP == getch()) {
    Serial.print((char)STK_INSYNC);
    Serial.print((char)STK_OK);
  } else {
    set_error( 1, 0 );
    Serial.print((char)STK_NOSYNC);
  }
}

/*----------------------------------------------------------------------------*/

void breply(uint8_t b) {
  if (CRC_EOP == getch()) {
    Serial.print((char)STK_INSYNC);
    Serial.print((char)b);
    Serial.print((char)STK_OK);
  } 
  else {
    set_error( 2, 0 );
    Serial.print((char)STK_NOSYNC);
  }
}

/*----------------------------------------------------------------------------*/

void get_version(uint8_t c) {
  switch(c) {
  case 0x80:
    breply(HWVER);
    break;
  case 0x81:
    breply(SWMAJ);
    break;
  case 0x82:
    breply(SWMIN);
    break;
  case 0x93:
    breply('S'); // serial programmer
    break;
  default:
    breply(0);
  }
}

/*----------------------------------------------------------------------------*/

void set_parameters() 
{
  // call this after reading paramter packet into buff[]
  param.devicecode = buff[0];
  param.revision = buff[1];
  param.progtype = buff[2];
  param.parmode = buff[3];
  param.polling = buff[4];
  param.selftimed = buff[5];
  param.lockbytes = buff[6];
  param.fusebytes = buff[7];
  param.flashpoll = buff[8]; 
  // ignore buff[9] (= buff[8])
  // following are 16 bits (big endian)
  param.eeprompoll = beget16(&buff[10]);
  param.pagesize = beget16(&buff[12]);
  param.eepromsize = beget16(&buff[14]);

  // 32 bits flashsize (big endian)
  param.flashsize = 
      buff[16] * 0x01000000
      + buff[17] * 0x00010000
      + buff[18] * 0x00000100
      + buff[19];
}

/*----------------------------------------------------------------------------*/

void start_pmode() 
{
  // Tuning clock *must* be turned off before SCK is turned into an output because T0 on the 8-pin processors is also SCK
  #if EXTRA_OUTPUT_TUNING_CLOCK
    stop_tuning_clock();
  #endif

  spi_init();

/* rmv
  // fix: Drive RESET LOW before mucking with SCK?
  // http://code.google.com/p/mega-isp/issues/detail?id=22

  // following delays may not work on all targets...
  pinMode( RESET, OUTPUT );
  digitalWrite( RESET, HIGH );
  pinMode( SCK, OUTPUT );
  digitalWrite( SCK, LOW );
  delay( 50 );
  digitalWrite( RESET, LOW );
  delay( 50 );
*/
  pinMode( RESET, OUTPUT );
  digitalWrite( RESET, LOW );

  pinMode( SCK, OUTPUT );
  digitalWrite( SCK, LOW );

  delay( 50 );

  pinMode( MISO, INPUT );
  pinMode( MOSI, OUTPUT );

  // fix: Check the value returned from the processor.  Ensure it entered programming mode.
  spi_transaction( 0xAC, 0x53, 0x00, 0x00 );

  pmode = 1;
}

/*----------------------------------------------------------------------------*/

void end_pmode() 
{
  pinMode( MISO, INPUT );
  pinMode( MOSI, INPUT );

  #if ! PROGRAMMER_USE_SLOW_SPI_CLOCK
    SPCR &= ~ (1 << SPE);
  #endif

  #if RELAY_ENABLED
    if ( HoldInResetAfterProgramming )
    {
      HeldInReset = true;
    }
    else
    {
      pinMode( SCK, INPUT );
      pinMode( RESET, INPUT );
      HeldInReset = false;
    }
  #else
    pinMode( SCK, INPUT );
    pinMode( RESET, INPUT );
  #endif

  pmode = 0;

  // Don't turn the tuning clock on until after SCK is turned into an input because T0 on the 8-pin processors is also SCK
  #if EXTRA_OUTPUT_TUNING_CLOCK
    start_tuning_clock();
  #endif
}

/*----------------------------------------------------------------------------*/

void universal() 
{
//rmv  int w;
  uint8_t ch;

  fill(4);
  ch = spi_transaction(buff[0], buff[1], buff[2], buff[3]);
  breply(ch);
}

/*----------------------------------------------------------------------------*/

void flash(uint8_t hilo, int addr, uint8_t data) 
{
  spi_transaction(0x40+8*hilo, 
      addr>>8 & 0xFF, 
      addr & 0xFF,
      data);
}

/*----------------------------------------------------------------------------*/

void commit(int addr) 
{
/* rmv
  static bool JustOnce = true;
*/
  #if PROGRAMMER_USE_ONE_LED
    uint8_t RdyBsy;
  #endif

/* rmv
  if ( JustOnce )
  {
    JustOnce = false;
    pinMode( 3, OUTPUT );
    digitalWrite( 3, HIGH );
  }
*/

  #if PROGRAMMER_USE_ONE_LED
  #else
    if (PROG_FLICKER) prog_lamp(LOW);
  #endif
  
  spi_transaction( 0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0 );

/* rmv 
  RdyBsy = spi_transaction( 0xF0, 0x00, 0x00, 0x00 );

  if ( (RdyBsy & 0x01) != 0x01 )
  {
    digitalWrite( 3, LOW );
  }
*/

  #if PROGRAMMER_USE_ONE_LED
/*
    delay( 30 );
*/
    {
      TheLED.flash();
      unsigned long Start;
      Start = millis();
      while ( millis() - Start < 30 )
      {
        TheLED.update();
        if ( param.polling )
        {
          RdyBsy = spi_transaction( 0xF0, 0x00, 0x00, 0x00 );
          if ( (RdyBsy & 0x01) == 0x00 )
            break;
        }
      }
    }
  #else
    if (PROG_FLICKER) 
    {
      delay(PTIME);
      prog_lamp(HIGH);
    }
  #endif

/* rmv
  RdyBsy = spi_transaction( 0xF0, 0x00, 0x00, 0x00 );

  if ( (RdyBsy & 0x01) == 0x01 )
  {
    digitalWrite( 3, LOW );
  }
*/
}

/*----------------------------------------------------------------------------*/

//#define _current_page(x) (here & 0xFFFFE0)
int current_page(int addr) {
  if (param.pagesize == 32) return here & 0xFFFFFFF0;
  if (param.pagesize == 64) return here & 0xFFFFFFE0;
  if (param.pagesize == 128) return here & 0xFFFFFFC0;
  if (param.pagesize == 256) return here & 0xFFFFFF80;
  return here;
}

/*----------------------------------------------------------------------------*/

uint8_t write_flash_pages(int length) {
  int x = 0;
  int page = current_page(here);
  while (x < length) {
    if (page != current_page(here)) {
      commit(page);
      page = current_page(here);
    }
    flash(LOW, here, buff[x++]);
    flash(HIGH, here, buff[x++]);
    here++;
  }

  commit(page);

  return STK_OK;
}

/*----------------------------------------------------------------------------*/

void write_flash(int length) {
  fill(length);
  if (CRC_EOP == getch()) {
    Serial.print((char) STK_INSYNC);
    Serial.print((char) write_flash_pages(length));
  } else {
    set_error( 3, 0 );
    Serial.print((char) STK_NOSYNC);
  }
}

/*----------------------------------------------------------------------------*/

// write (length) bytes, (start) is a byte address
uint8_t write_eeprom_chunk(int start, int length) 
{
  // this writes byte-by-byte,
  // page writing may be faster (4 bytes at a time)
  fill(length);

  #if PROGRAMMER_USE_ONE_LED
  #else
    prog_lamp(LOW);
  #endif

  for (int x = 0; x < length; x++) 
  {
    int addr = start+x;
    spi_transaction(0xC0, (addr>>8) & 0xFF, addr & 0xFF, buff[x]);
    delay(45);
  }

  #if PROGRAMMER_USE_ONE_LED
    TheLED.flash();
  #else
    prog_lamp(HIGH); 
  #endif

  return STK_OK;
}

/*----------------------------------------------------------------------------*/

#define EECHUNK (32)
uint8_t write_eeprom(int length) {
  // here is a word address, get the byte address
  int start = here * 2;
  int remaining = length;
  if (length > param.eepromsize) {
    set_error( 4, 0 );
    return STK_FAILED;
  }
  while (remaining > EECHUNK) {
    write_eeprom_chunk(start, EECHUNK);
    start += EECHUNK;
    remaining -= EECHUNK;
  }
  write_eeprom_chunk(start, remaining);
  return STK_OK;
}

/*----------------------------------------------------------------------------*/

void program_page() {
  char result = (char) STK_FAILED;
  int length;
  char memtype;
  
  length = 256 * getch();
  length = length | getch();
  memtype = getch();
  
  // flash memory @here, (length) bytes
  if (memtype == 'F') {
    write_flash(length);
    return;
  }
  if (memtype == 'E') {
    result = (char)write_eeprom(length);
    if (CRC_EOP == getch()) {
      Serial.print((char) STK_INSYNC);
      Serial.print(result);
    } else {
      set_error( 5, 0 );
      Serial.print((char) STK_NOSYNC);
    }
    return;
  }
  Serial.print((char)STK_FAILED);
  return;
}

/*----------------------------------------------------------------------------*/

uint8_t flash_read(uint8_t hilo, int addr) {
  return spi_transaction(0x20 + hilo * 8,
    (addr >> 8) & 0xFF,
    addr & 0xFF,
    0);
}

/*----------------------------------------------------------------------------*/

char flash_read_page(int length) {
  for (int x = 0; x < length; x+=2) {
    uint8_t low = flash_read(LOW, here);
    Serial.print((char) low);
    uint8_t high = flash_read(HIGH, here);
    Serial.print((char) high);
    here++;
  }
  return STK_OK;
}

/*----------------------------------------------------------------------------*/

char eeprom_read_page(int length) {
  // here again we have a word address
  int start = here * 2;
  for (int x = 0; x < length; x++) {
    int addr = start + x;
    uint8_t ee = spi_transaction(0xA0, (addr >> 8) & 0xFF, addr & 0xFF, 0xFF);
    Serial.print((char) ee);
  }
  return STK_OK;
}

/*----------------------------------------------------------------------------*/

void read_page() {
  char result = (char)STK_FAILED;
  int length;
  char memtype;
  
  length = 256 * getch();
  length = length | getch();
  memtype = getch();
  
  if (CRC_EOP != getch()) {
    set_error( 6, 0 );
    Serial.print((char) STK_NOSYNC);
    return;
  }
  Serial.print((char) STK_INSYNC);
  if (memtype == 'F') result = flash_read_page(length);
  if (memtype == 'E') result = eeprom_read_page(length);
  Serial.print(result);
  
  #if PROGRAMMER_USE_ONE_LED
    TheLED.flash();
  #endif
}

/*----------------------------------------------------------------------------*/

void read_signature() {
  if (CRC_EOP != getch()) {
    set_error( 7, 0 );
    Serial.print((char) STK_NOSYNC);
    return;
  }
  Serial.print((char) STK_INSYNC);
  uint8_t high = spi_transaction(0x30, 0x00, 0x00, 0x00);
  Serial.print((char) high);
  uint8_t middle = spi_transaction(0x30, 0x00, 0x01, 0x00);
  Serial.print((char) middle);
  uint8_t low = spi_transaction(0x30, 0x00, 0x02, 0x00);
  Serial.print((char) low);
  Serial.print((char) STK_OK);
}

/*----------------------------------------------------------------------------*/

void avrisp() { 
  uint8_t data, low, high;
  uint8_t ch = getch();
  
#if RELAY_ENABLED
  if ( (ch == '!') && ! pmode )
  {
    do_serial_relay();
  }
  else
#endif
  {
    switch (ch) {

    case '0': // signon
      ++command_count;
      error_count = 0;
      first_mark = 0;
      empty_reply();
      break;

    case '1':
      ++command_count;
      if (getch() == CRC_EOP) {
        Serial.print((char) STK_INSYNC);
        Serial.print("AVR ISP");
        Serial.print((char) STK_OK);
      }
      break;

    case 'A':
      ++command_count;
      get_version(getch());
      break;

    case 'B':
      ++command_count;
      fill(20);
      set_parameters();
      empty_reply();
      break;

    case 'E': // extended parameters - ignore for now
      ++command_count;
      fill(5);
      empty_reply();
      break;
  
    case 'P':
      ++command_count;
      start_pmode();
      empty_reply();
      break;

    case 'U': // set address (word)
      ++command_count;
      here = getch();
      here = here | (256 * getch());
      empty_reply();
      break;
  
    case 0x60: //STK_PROG_FLASH
      ++command_count;
      low = getch();
      high = getch();
      empty_reply();
      break;

    case 0x61: //STK_PROG_DATA
      ++command_count;
      data = getch();
      empty_reply();
      break;
  
    case 0x64: //STK_PROG_PAGE
      ++command_count;
      program_page();
      break;
      
    case 0x74: //STK_READ_PAGE 't'
      ++command_count;
      read_page();    
      break;
  
    case 'V': //0x56
      ++command_count;
      universal();
      break;

    case 'Q': //0x51
      ++command_count;
      error_count = 0;
      first_mark = 0;
      end_pmode();
      empty_reply();
      break;
      
    case 0x75: //STK_READ_SIGN 'u'
      ++command_count;
      read_signature();
      break;
  
    // expecting a command, not CRC_EOP
    // this is how we can get back in sync
    case CRC_EOP:
      set_error( 8, 0 );
      Serial.print((char) STK_NOSYNC);
      break;
      
    // anything else we will return STK_UNKNOWN
    default:
      set_error( 9, ch );
      if (CRC_EOP == getch()) 
        Serial.print((char)STK_UNKNOWN);
      else
        Serial.print((char)STK_NOSYNC);
    }
  }
}

/*----------------------------------------------------------------------------*/

void loop(void) 
{
  #if PROGRAMMER_USE_ONE_LED
  #else
    // is pmode active?
    if (pmode) digitalWrite(LED_PMODE, HIGH); 
    else digitalWrite(LED_PMODE, LOW);
  #endif

  #if PROGRAMMER_USE_ONE_LED
    if ( error_count ) TheLED.error();
  #else
    // is there an error?
    if (error_count) digitalWrite(LED_ERR, HIGH); 
    else digitalWrite(LED_ERR, LOW);
  #endif
  
  #if PROGRAMMER_USE_ONE_LED
    TheLED.update();
  #else
    // light the heartbeat LED
    heartbeat();
  #endif

  if (Serial.available()) {
    avrisp();
  }
}

/*----------------------------------------------------------------------------*/
