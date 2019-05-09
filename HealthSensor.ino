#include <CapacitiveSensor.h>
#include <Wire.h>
#define SOURCEPIN     4
#define OXYUPPIN      10
#define OXYDOWNPIN    12
#define ADRENUPPIN    9
#define ADRENDOWNPIN  7
#define I2CADDRESS    26
#define FRONT         300
/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 */


CapacitiveSensor   oxyUpSensor = CapacitiveSensor(SOURCEPIN,OXYUPPIN);            // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
CapacitiveSensor   oxyDownSensor = CapacitiveSensor(SOURCEPIN,OXYDOWNPIN);        // 10M resistor between pins 4 & 6, pin 6 is sensor pin, add a wire and or foil
CapacitiveSensor   adrenUpSensor = CapacitiveSensor(SOURCEPIN,ADRENUPPIN);        // 10M resistor between pins 4 & 8, pin 8 is sensor pin, add a wire and or foil
CapacitiveSensor   adrenDownSensor = CapacitiveSensor(SOURCEPIN,ADRENDOWNPIN);    // 10M resistor between pins 4 & 8, pin 8 is sensor pin, add a wire and or foil

boolean oxyUpState = false;
boolean oxyDownState = false;
boolean adrenUpState = false;
boolean adrenDownState = false;

boolean oxyUpWait = false;
boolean oxyDownWait = false;
boolean adrenUpWait = false;
boolean adrenDownWait = false;


void setup()                    
{
   Serial.begin(9600);
   Wire.begin(I2CADDRESS);
   Wire.onRequest(requestEvent);
}

void loop()                    
{
    long oxyUp =  oxyUpSensor.capacitiveSensor(30);
    long oxyDown =  oxyDownSensor.capacitiveSensor(30);
    long adrenUp =  adrenUpSensor.capacitiveSensor(30);
    long adrenDown =  adrenDownSensor.capacitiveSensor(30);
    
    //DEBUG
    Serial.println("OU = " + String(oxyUp) + "\tOD = " + String(oxyDown) + "\tAU = " + String(adrenUp) + "\tAD = " + String(adrenDown));
    //DEBUG
    
    if(oxyUp > FRONT) oxyUpWait = true;
    if(oxyDown > FRONT) oxyDownWait = true;
    if(adrenUp > FRONT) adrenUpWait = true;
    if(adrenDown > FRONT) adrenDownWait = true;

    if(oxyUp < FRONT && oxyUpWait) { oxyUpState = true; oxyUpWait = false; }
    if(oxyDown < FRONT && oxyDownWait) { oxyDownState = true; oxyDownWait = false; }
    if(adrenUp < FRONT && adrenUpWait) { adrenUpState = true; adrenUpWait = false; }
    if(adrenDown < FRONT && adrenDownWait) { adrenDownState = true; adrenDownWait = false; }

    delay(20);
}

void requestEvent()
{
  byte unswer = 0x00;//B00000000
  if(oxyUpState) { unswer = unswer | (1 << 7); oxyUpState = false; }
  if(oxyDownState) { unswer = unswer | (1 << 5); oxyDownState = false; }
  if(adrenUpState) { unswer = unswer | (1 << 3); adrenUpState = false; }
  if(adrenDownState) { unswer = unswer | (1 << 1); adrenDownState = false; }
  Wire.write(unswer);
}

