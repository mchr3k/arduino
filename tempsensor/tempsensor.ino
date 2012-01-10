#include "StringReader.h"

StringReader StrReader;

// Buffer which we read data into
const int STR_DATA_LEN = 50;
char stringData[STR_DATA_LEN];

// 7 minutes * 1440 readings = 7 days
const unsigned long TEMP_INTERVAL = 420000; // 7 * 60 * 1000
// 1 bytes per reading so memory usage is NUM_TEMP_READINGS
const int NUM_TEMP_READINGS = 1440;
unsigned char tempData[NUM_TEMP_READINGS];
int tempIndex = 0;
int numReadings = 0;
int totalReadings = 0;
unsigned long SERIAL_SPEED = 115200;

unsigned long nowTime = 0;
unsigned long ledOffTime = 0;
unsigned long nextReadingTime = 0;

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(SERIAL_SPEED);  //Start the serial connection with the copmuter
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
  Serial.println("CollectingData");
  
  while (true)
  {
    // Wait for the next reading time
    unsigned long nowTime = millis();
    if (nowTime <= ledOffTime) { digitalWrite(13, HIGH); }
    bool loopBreak = false;
    bool ledOn = true;
    while (nowTime < nextReadingTime)
    {
      if (ledOn && (nowTime > ledOffTime)) { digitalWrite(13, LOW); ledOn = false; }      
      if (StrReader.readString(stringData, STR_DATA_LEN) > 0) { loopBreak = true; break; }
      delay(100);
      nowTime = millis();
    }
    if (loopBreak) {break;}
    
    //Serial.print("Collect Data - Index ");
    //Serial.prinln(tempIndex);
    unsigned char latestTemp = (unsigned char)(getTemp() * 5);
    tempData[tempIndex] = latestTemp;
    tempIndex++;
    totalReadings++;
    
    // Schedule next reading
    ledOffTime = nowTime + 1000;
    nextReadingTime = nowTime + TEMP_INTERVAL;
    
    if (numReadings < NUM_TEMP_READINGS) { numReadings++; }   
    if (tempIndex == NUM_TEMP_READINGS) { tempIndex = 0; }
  }
  
  Serial.println("GotData");
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
  Serial.println("------");
  Serial.print("Total Readings, ");
  Serial.println(totalReadings);
  Serial.print("Num Readings, ");
  Serial.println(numReadings);
  Serial.print("Lost Readings, ");
  Serial.println((totalReadings - numReadings));
  Serial.print("Reading Interval, ");
  Serial.println(TEMP_INTERVAL);
  Serial.println("------");
  digitalWrite(13, HIGH);
  while(outputCount < numReadings)
  {    
    if (i >= NUM_TEMP_READINGS)
    {
      i = 0;
    }
    Serial.print(i);
    Serial.print(",");
    double dtempData = (double)tempData[i];
    double fixdtempData = dtempData / 5;
    Serial.println(fixdtempData);
    outputCount++;
    i++;
  }
  digitalWrite(13, LOW);
  Serial.println("------");
  
  if (strcmp(stringData, "reset") == 0)
  {    
    tempIndex = 0;
    numReadings = 0;
    totalReadings = 0;
    Serial.println("Reset");
    Serial.println("------");
  }
}

