#include "Wire.h"
#define UPBUTTON 3
#define DOWNBUTTON 4

boolean upButtState = true;
boolean downButtState = true;
int slaveAddress = 22;
int videoNumber = 4;
void setup() {
  Serial.begin(9600);
  Wire.begin(slaveAddress);
  pinMode(UPBUTTON, INPUT_PULLUP);
  pinMode(DOWNBUTTON, INPUT_PULLUP);
  Wire.onRequest(requestEvent);
}

void loop() {
  if (downButtState && !digitalRead(DOWNBUTTON))
  {
    delay(5);//Кнопка перехода уровня.
    if (!digitalRead(DOWNBUTTON))
    {
      videoNumber++;
      if (videoNumber > 28) videoNumber = 0;
      Serial.println("#S|START" + String(videoNumber) + "|[]#"); // "#S|START0|[]#"
    }
  }
  downButtState = digitalRead(DOWNBUTTON);

  if (upButtState && !digitalRead(UPBUTTON))
  {
    delay(5);//Кнопка перехода уровня.
    if (!digitalRead(UPBUTTON))
    {
      videoNumber--;
      if (videoNumber < 0) videoNumber = 28;
      Serial.println("#S|START" + String(videoNumber) + "|[]#"); // "#S|START0|[]#
    }
  }
  upButtState = digitalRead(UPBUTTON);
}

void requestEvent()
{
  Wire.write(videoNumber & 0xFF);
}

