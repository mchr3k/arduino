const int potentPin = A0;
const int potentLed = 5;

const int lightPin  = A1;
const int lightLed  = 6;

const int blueLed   = 9;
const int greenLed  = 10;
const int redLed    = 11;

const int inputPin1 = 3;

// Range 0 -> 3
// 0 - single LED
// 1-3 - 3 colors of LED
int selectedLed = 0;
int singleVal = 0;
int blueVal = 0;
int greenVal = 0;
int redVal = 0;

void setup() 
{
  // declare the potentLed as an OUTPUT:
  pinMode(potentLed, OUTPUT);
  pinMode(lightLed, OUTPUT);
  
  pinMode(inputPin1, INPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  Serial.begin(9600);
}

long lastDebounceTime = 0;
long debounceDelay = 50;
int lastButtonState = LOW;
int pressHandled = 0;
int active = 0;

int blueInc = 1;
int greenInc = 3;
int redInc = 5;

void loop() 
{
  // Read button
  int reading = digitalRead(inputPin1);  
  if (reading != lastButtonState) 
  {
    lastDebounceTime = millis();    
  }   
  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if ((reading == LOW) && (pressHandled == 0))
    {
      /*selectedLed++;
      if (selectedLed == 0) { selectedLed++; }
      if (selectedLed > 3) selectedLed = 0;*/
      
      if (active == 0) active = 1;
      else if (active == 1) active = 0;
      pressHandled = 1;
    }
    else if (reading == HIGH)
    {
      pressHandled = 0;
    }
  }
  lastButtonState = reading;
  
  if (active == 1)
  {
    animate(&blueVal, &blueInc);
    animate(&greenVal, &greenInc);
    animate(&redVal, &redInc);    
    
    int value = analogRead(potentPin) / 4;
    value = map(value, 0, 255, 0, 100);
    value = constrain(value, 0, 100);
    delay(value);
  }

  analogWrite(blueLed, blueVal);
  analogWrite(greenLed, greenVal);
  analogWrite(redLed, redVal);
  
  // Handle Potent reading
  /*int value = analogRead(potentPin) / 4;
  if (selectedLed == 0) { singleVal = value; }
  else if (selectedLed == 1) { blueVal = value; }
  else if (selectedLed == 2) { greenVal = value; }
  else if (selectedLed == 3) { redVal = value; }
  
  // Update LEDs
  analogWrite(potentLed, 0  );
  //analogWrite(potentLed, singleVal);
  analogWrite(blueLed, blueVal);
  analogWrite(greenLed, greenVal);
  analogWrite(redLed, redVal);
  
  // Handle Light LED
  //int lightLevel = analogRead(lightPin);
  //lightLevel = map(lightLevel, 100, 800, 0, 255);
  //lightLevel = constrain(lightLevel, 0, 255);
  //analogWrite(lightLed, lightLevel);
  analogWrite(lightLed, 0);*/
  
  // Debug
  /*Serial.println(singleVal);
  Serial.println(blueVal);
  Serial.println(greenVal);
  Serial.println(redVal);
  Serial.println("===");
  delay(1000);*/
}

void animate(int* value, int* increment)
{
  *value += *increment;
  if (*value > 255) 
  {
    *value -= *value - 255;
    *increment *= -1;
  }
  else if (*value < 0)
  {
    *value *= -1;
    *increment *= -1;
  }
}
