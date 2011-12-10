#include <SoftwareSerial.h>
#include "MultiSerial.h"
#include "StringReader.h"

MultiSerial MSerial(true);
StringReader StrReader;

// Buffer which we read data into
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];

// 4 minutes * 360 readings = 24 hours
const unsigned long TEMP_INTERVAL = 1000;//4 * 60 * 1000;
// 4 bytes per float so memory usage is 4 * NUM_TEMP_READINGS
const int NUM_TEMP_READINGS = 360;
float tempData[NUM_TEMP_READINGS];
int tempIndex = 0;
int numReadings = 0;
int totalReadings = 0;
unsigned long SERIAL_SPEED = 115200;

void setup()
{
  pinMode(13, OUTPUT);
  MSerial.begin(SERIAL_SPEED);  //Start the serial connection with the copmuter
                                //to view the result open the serial monitor 
                                //last button beneath the file bar (looks like a box with an antenae)
}
 
void loop()
{
  recordTempData();
  returnTempData();
}

void recordTempData()
{
  MSerial.println("CollectingData");
  
  while (true)
  {
    //MSerial.print("Collect Data - Index ");
    //MSerial.prinln(tempIndex);
    float latestTemp = getTemp();
    tempData[tempIndex] = latestTemp;
    tempIndex++;
    totalReadings++;
    
    if (numReadings < NUM_TEMP_READINGS) { numReadings++; }   
    if (tempIndex == NUM_TEMP_READINGS) { tempIndex = 0; }
    if (StrReader.readString(&MSerial, stringData, STR_DATA_LEN) > 0) { break; }
    
    digitalWrite(13, HIGH);    
    delay(TEMP_INTERVAL / 2);
    digitalWrite(13, LOW);
    delay(TEMP_INTERVAL / 2);
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
  int i = tempIndex + 1;       
  if (numReadings < NUM_TEMP_READINGS)
  {
    // Got some blank values - skip these
    i += (NUM_TEMP_READINGS - numReadings);
  }
  MSerial.println("------");
  MSerial.print("Total Readings, ");
  MSerial.println(totalReadings);
  if (numReadings < totalReadings)
  {
    MSerial.print("Lost Readings, ");
    MSerial.println((totalReadings - numReadings));
  }
  MSerial.print("Reading Interval, ");
  MSerial.println(TEMP_INTERVAL);
  MSerial.println("------");
  while(outputCount < numReadings)
  {
    delay(50);
    digitalWrite(13, HIGH);
    if (i >= NUM_TEMP_READINGS)
    {
      i = 0;
    }
    MSerial.print(i);
    MSerial.print(",");
    MSerial.println(tempData[i]);
    outputCount++;
    i++;
    delay(50);
    digitalWrite(13, LOW);
  }
  MSerial.println("------");
  
  if (strcmp(stringData, "reset") == 0)
  {    
    tempIndex = 0;
    numReadings = 0;
    totalReadings = 0;
    MSerial.println("Reset");
    MSerial.println("------");
  }
}

