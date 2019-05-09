/*
   1. Сбор данных с андроида
   2. Сбор данных с ардуино видео-процессинга
   3. чтение сенсорных кнопок и управление Led.
*/
#include <Wire.h>
#include <LedControl.h>
#include <FastLED.h>
#include <SoftwareSerial.h>

//PINS
#define DPIN 12
#define CLKPIN 11
#define CSPIN 10

#define ANDROIDRX   9
#define ANDROIDTX   8
#define STRIPPIN    6
#define WINRELAY    4

//I2C Address
#define VIDEOADDR   22
#define SENSORADDR  26

//Keys of OXY and ADRENALINE
#define OXYUP     1
#define OXYDOWN   2
#define ADRENUP   4
#define ADRENDOWN 3

#define NUM_LEDS 12

LedControl leds = LedControl(DPIN, CLKPIN, CSPIN, 2); // Инициализация MAX7221
CRGB Strip[NUM_LEDS];
SoftwareSerial Android(ANDROIDRX, ANDROIDTX);//Инициализация Serial-соединения с Android Mixer
int colors[17][3] =
{
  {110, 160, 165},
  {50, 140, 205},
  {255, 0, 180},
  {0, 200, 40},
  {130, 220, 250},
  {100, 0, 90},
  {130, 0, 255},
  {255, 125, 15},
  {40, 100, 200},
  {0, 130, 100},
  {25, 250, 130},
  {0, 240, 130},
  {80, 0, 20},
  {0, 90, 150},
  {0, 200, 30},
  {255, 210, 190},
  {0, 230, 190}
};


int NAValues[29] = {55, 14, 25, 67, 83, 100, 91, 18, 21, 69, 23, 38, 36, 99, 97, 12, 97, 83, 87, 75, 73, 57, 87, 28, 39, 42, 89, 18, 37};
int HRValues[17] = {180, 98, 57, 220, 122, 176, 97, 63, 73, 84, 61, 77, 89, 144, 167, 122, 138};
int HRwinIndex = 3; // Необходимая позиция NAValues для победы.
int NAwinIndex = 26; // Необходимая позиция HRValues для победы.
int HeartRate, NeuroActivity, Adrenaline, Oxygen, prevNeuroActivity, prevHeartRate;
boolean HR, NA, A, O, WIN, game;


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Android.begin(9600);
  
  leds.shutdown(0, false);
  leds.setIntensity(0, 15);
  leds.clearDisplay(0);
  leds.shutdown(1, false);
  leds.setIntensity(1, 15);
  leds.clearDisplay(1);

  FastLED.setBrightness(40);
  FastLED.addLeds<NEOPIXEL, STRIPPIN>(Strip, NUM_LEDS);
  FastLED.setBrightness(40);
  for (int i = 0; i < NUM_LEDS; i++) Strip[i] = CRGB::Blue;
  FastLED.show();
  
  pinMode(WINRELAY, OUTPUT);
  digitalWrite(WINRELAY, HIGH);
  
  prevNeuroActivity = 0;
  prevHeartRate = HRValues[5];
  HeartRate = HRValues[5];
  NeuroActivity = 0;
  Adrenaline = 30;
  Oxygen = 30;
  setLeds();
  HR = false;
  O = false;
  NA = false;
  A = false;
  game = false;
  WIN = false;
}

void loop()
{
  if (game)
  {
    if(WIN)
    {
      delay(4000);
      Serial.println("END");
      digitalWrite(WINRELAY, HIGH);
      while(true);
    }
    readHeartRate(); //Android
    readNeuroActivity(); //VideoPlayer
    readSensors(); //Buttons
    setLeds(); //BarUpdate
    checkWin();
    sendToProcessing(); //Send all data to HealthMonitor...
    delay(200);
  }
  else connectToMonitor();
}

int readAndroid()
{
  int hrVal = -1;
  if (Android.available() > 0)
  {
    String hrStr = Android.readStringUntil('\n');
    if (hrStr.startsWith("MIXER=")) hrVal = hrStr.substring(6, hrStr.length()).toInt();
    //Serial.println("MIXER DATA = " + String(hrVal));
    return hrVal;
  }
  return hrVal;
}

int readVideoSlave()
{
  int videoIndex = -1;
  byte unswerCnt = Wire.requestFrom(VIDEOADDR, 1, false);
  if (unswerCnt == 1) videoIndex = (int)Wire.read();
  //Serial.println("Video Index = " + String(videoIndex));
  return videoIndex;
}

void readSensors()
{
  int keyNumber = -1;
  byte unswerCnt = Wire.requestFrom(SENSORADDR, 1, false);
  if (unswerCnt == 1)
  {
    byte unswer = Wire.read();
    //Serial.print("SENSOR STATES is ");
    //Serial.print(unswer,BIN);
    //Serial.println();
    if((unswer & (1 << 7)) > 0) { Oxygen -= 10; }
    if((unswer & (1 << 5)) > 0) { Oxygen += 10; }
    if((unswer & (1 << 3)) > 0) { Adrenaline += 10; }
    if((unswer & (1 << 1)) > 0) { Adrenaline -= 10; }
  }
  
  if(Oxygen > 100) Oxygen = 100;
  if(Oxygen < 0) Oxygen = 0;
  if(Adrenaline > 100) Adrenaline = 100;
  if(Adrenaline < 0) Adrenaline = 0;
  
  O = (Oxygen == 0);
  //Serial.println("OxyState is " + String(O,BIN));
  A = (Adrenaline == 100);
  //Serial.println("Adrenaline is " + String(A,BIN));
}

void readHeartRate()
{
  int HRIndex = readAndroid();
  //Serial.println("Heart Rate (MIXER) Index from Android = " + String(HRIndex));
  //Serial.print("HRIndex=" + String(HRIndex));
  if (HRIndex >= 0) 
  {
    HeartRate = HRValues[HRIndex];
    HR = (HRIndex == HRwinIndex);
  }
  if (prevHeartRate != HeartRate) 
  {
    getLow();
    //Serial.println(" and PrevHR=" + String(prevHeartRate) + " and HR = " + String(HeartRate));
    int r = colors[HRIndex][0];
    int g = colors[HRIndex][1];
    int b = colors[HRIndex][2];
    for (int i = 0; i < NUM_LEDS; i++) Strip[i].setRGB(r,g,b);
    FastLED.show();
    prevHeartRate = HeartRate;
  }
  
  //Serial.println("Heart Rate = " + String(HeartRate));
}

void readNeuroActivity()
{
  int NAIndex = readVideoSlave();
  if (NAIndex >= 0) 
  {
    NeuroActivity = NAValues[NAIndex];
    NA = (NAIndex == NAwinIndex);
  }
  if (prevNeuroActivity != NeuroActivity) getLow();
  prevNeuroActivity = NeuroActivity;
  //Serial.println("Neuro Activity = " + String(NeuroActivity));
}

void getLow()
{
  Adrenaline = 30;
  Oxygen = 30;
  //Serial.println("GetLow Adrenaline and Oxygen onFirstandSecondSlaveChanged");
}

void sendToProcessing()
{
  //Serial.println("Sending to processing:");
  Serial.println(String(prevHeartRate) + ";" + String(prevNeuroActivity) + ";" +
                 String(Adrenaline) + ";" + String(Oxygen) + ";");
}

void connectToMonitor()
{
  if (Serial.available() > 0)
  {
    String input = "";
    input = Serial.readStringUntil('\n');
    if (input.startsWith("letsgame")) {
      game = true;
      Serial.println("start");
      delay(3000);
      digitalWrite(13, HIGH);
    }
  }
}

void setLeds()
{
  int AdreLeds = Adrenaline / 10;
  int OxyLeds = (100 - Oxygen) / 10;
  //Serial.println("Adrenaline ledCount = " + String(AdreLeds));
  //Serial.println("Oxygen ledCount = " + String(OxyLeds));
  if (AdreLeds >= 0 && AdreLeds < 11)
  {
    byte LEDBit = B11111111;
    if (AdreLeds <= 8)
    { 
      LEDBit = LEDBit << (8 - AdreLeds); 
      leds.setRow(0, 0, LEDBit); 
      leds.setRow(0, 1, B00000000); 
    }
    else
    { 
      LEDBit = LEDBit << (16 - AdreLeds); 
      leds.setRow(0, 0, B11111111); 
      leds.setRow(0, 1, LEDBit);
    }
  }
  
  if (OxyLeds >= 0 && OxyLeds < 11)
  {
    byte LEDBit = B11111111;
    if (OxyLeds <= 8)
    { 
      LEDBit = LEDBit << (8 - OxyLeds); 
      leds.setRow(1, 0, LEDBit); 
      leds.setRow(1, 1, B00000000); 
    }
    else
    { 
      LEDBit = LEDBit << (16 - OxyLeds); 
      leds.setRow(1, 0, B11111111);
      leds.setRow(1, 1, LEDBit);
    }
  }
}

void checkWin()
{
  if(O && A && NA && HR) 
  {
    HRValues[0] = 240;
    prevHeartRate = HRValues[0];
    HeartRate = HRValues[0];
    NAValues[26] = 100;
    NeuroActivity = NAValues[26];
    prevNeuroActivity = NAValues[26];
    WIN = true;
  }
}

