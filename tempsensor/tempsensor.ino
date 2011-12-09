#include "Print.h"
#include <SoftwareSerial.h>

class MultiSerial : public Print
{
  private:
    SoftwareSerial softSerial;
  public:
    MultiSerial();
    void begin(unsigned long);
    virtual int available();
    virtual int read();
    virtual size_t write(uint8_t);
};

MultiSerial::MultiSerial () : softSerial(2,3) {}

void MultiSerial::begin(unsigned long serSpeed)
{
  Serial.begin(serSpeed);
  softSerial.begin(serSpeed);
}
int MultiSerial::available()
{
  return softSerial.available();
}
int MultiSerial::read()
{
  return softSerial.read();
}
size_t MultiSerial::write(uint8_t c)
{
  softSerial.write(c);
  return Serial.write(c);
}

MultiSerial MSerial;

void setup()
{
  MSerial.begin(115200);  //Start the serial connection with the copmuter
                         //to view the result open the serial monitor 
                         //last button beneath the file bar (looks like a box with an antenae)
}
 
void loop()
{
  recordTempData();
  returnTempData();
}

const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];
char inChar=-1;
int instrIndex = 0;
const unsigned long MAX_SERIAL_WAIT = 100;

int readString()
{
  // Start at 0 so that we return quickly if there is no data initially
  unsigned long startTime = 0;
  instrIndex = 0;
  
  while(true)
  {
    if (MSerial.available() > 0)
    {
      startTime = millis();
      inChar = MSerial.read();
      stringData[instrIndex] = inChar;
      instrIndex++;
      stringData[instrIndex] = '\0';
    }
    
    unsigned long elapsedTime = millis() - startTime;
    if ((elapsedTime > MAX_SERIAL_WAIT) ||
        (instrIndex == (STR_DATA_LEN - 1)))
    {
      break;
    }
  }
  
  return instrIndex;
}

// 4 minutes * 360 readings = 24 hours
const unsigned long TEMP_INTERVAL = 4 * 60 * 1000;
// 4 bytes per float so memory usage is 4 * NUM_TEMP_READINGS
const int NUM_TEMP_READINGS = 360;
float tempData[NUM_TEMP_READINGS];
int tempIndex = 0;
int numReadings = 0;
int totalReadings = 0;

void recordTempData()
{
  MSerial.println("CollectingData");
  
  while (true)
  {
    //MSerial.print("Collect Data - Index ");
    //MSerial.println(tempIndex);
    float latestTemp = getTemp();
    tempData[tempIndex] = latestTemp;
    tempIndex++;
    totalReadings++;
    if (numReadings < NUM_TEMP_READINGS)
    {
      numReadings++;
    }   
    if (tempIndex == NUM_TEMP_READINGS)
    {
      tempIndex = 0;
    }
    if (readString() > 0) { break; }
    
    delay(TEMP_INTERVAL);
  }
  
  MSerial.println("GotData");
}

//TMP36 Pin Variables
int temperaturePin = 0; //the analog pin the TMP36's Vout (sense) pin is connected to
                        //the resolution is 10 mV / degree centigrade 
                        //(500 mV offset) to make negative temperatures an option

float getTemp()
{
 float temperature = getVoltage(temperaturePin);  //getting the voltage reading from the temperature sensor
 temperature = (temperature - .5) * 100;          //converting from 10 mv per degree wit 500 mV offset
                                                  //to degrees ((volatge - 500mV) times 100)
 return temperature;
}

/*
 * getVoltage() - returns the voltage on the analog input defined by
 * pin
 */
float getVoltage(int pin){
 return (analogRead(pin) * .004882814); //converting from a 0 to 1024 digital range
                                        // to 0 to 5 volts (each 1 reading equals ~ 5 millivolts
}

void returnTempData()
{
  int outputCount = 0;
  int i = tempIndex;        
  MSerial.println("------");
  MSerial.print("Total Readings: ");
  MSerial.println(totalReadings);
  if (numReadings < totalReadings)
  {
    MSerial.print("Lost Readings: ");
    MSerial.println((totalReadings - numReadings));
  }
  MSerial.print("Reading Interval: ");
  MSerial.println(TEMP_INTERVAL);
  while(outputCount < numReadings)
  {
    MSerial.print(i);
    MSerial.print(",");
    MSerial.println(tempData[i]);
    outputCount++;
    i++;
    if (i == NUM_TEMP_READINGS)
    {
      i = 0;
    }
  }
  MSerial.println("------");
}

