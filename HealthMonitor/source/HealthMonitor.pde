import processing.serial.*;

Serial arduino;
String portName;
PImage background;
int HeartRate, NeuroActivity, Adrenaline, Oxygen;
boolean HR_STATE, NA_STATE, A_STATE, O_STATE;
float scrH, scrW;
boolean game;
boolean connected;
int blue, red, white;
void setup()
{
  //Draw settings//
  background(0);
  //fullScreen();
  size(900,600);
  scrW = width;
  scrH = height;
  background = loadImage("back.jpg");
  blue = color(0,255,255);
  red = color(253,0,26);
  white = color(253,254,255);
  //Arduino connection manager//
  arduinoConnect();
  HR_STATE = false;
  NA_STATE = false;
  A_STATE = false;
  O_STATE = false;
  HeartRate = 240;
  NeuroActivity = 100;
  Adrenaline = 100;
  Oxygen = 80;
}

void draw()
{
  getUpdates();
  showBackground();
  showHeaders();
  showValues();
}

void arduinoConnect()
{
  connected = false;
  String[] comportConfig = loadStrings("com.txt");
  if(comportConfig.length > 0) portName = comportConfig[0].substring(comportConfig[0].indexOf("\"") + 1, comportConfig[0].length() - 2);
  arduino = new Serial(this, portName, 9600);
  long startConnect = millis();
  while (!connected || millis() - startConnect < 10000)
  {
    arduino.write("letsgame\n"); 
    long now = millis();
    while (millis() - now < 1000) {;}
    String input = getInput(); 
    if (input.length() > 4)
    {
      if (input.substring(0, 5).equals("start")) connected = true;
    }
  }
  if(!connected) { noLoop(); println("No connection for arduino..."); text("NO ARDUINO",width/2,height/2); }
}

String getInput()
{
  if (arduino.available() > 0)
  {
    String inp = arduino.readStringUntil('\n');
    if (inp != null)
    {
      if (inp.length() > 1) println(inp);
      return inp;
    } else return " ";
  } else return " ";
}

void getUpdates()
{
  String inData = getInput(); //"H;N;A;O;"//
  int[] Data = int(inData.split(";"));
  if(Data.length > 3)
  {
    HeartRate = Data[0];
    NeuroActivity = Data[1];
    Adrenaline = Data[2];
    Oxygen = Data[3];
  }
}

void showBackground()
{
  background(0);
  float backW = background.width;
  float backH = background.height;

  float backX, backY;
  float sizeX, sizeY;
  if (scrW/scrH > backW/backH)
  {
    sizeX = backW * (scrH/backH);
    sizeY = scrH;
    backX = (scrW - sizeX)/2;
    backY = 0;
  } else
  {
    sizeX = scrW;
    sizeY = backH * (scrW/backW);
    backX = 0;
    backY = (scrH - sizeY)/2;
  }
  image(background, backX, backY, sizeX, sizeY);
}

void showHeaders()
{
  textSize(30);
  fill(blue);
  textAlign(CENTER);
  text("NEURODOPE Health Monitor HM50021", scrW/2, scrH/16);
  fill(white);
  textSize(35);
  textAlign(LEFT);
  text("HEART RATE (BPM)", scrW/20, scrH/4);
  text("ADRENALINE LEVEL (%)", scrW/20, scrH/2 + scrH/8);
  textAlign(RIGHT);
  text("NEUROACTIVITY (% MG)", scrW - scrW/20, scrH/4);
  text("OXYGEN (CEREBRAL) (%)", scrW - scrW/20, scrH/2 + scrH/8);
}

void showValues()
{
  textSize(150);
  textAlign(CENTER);
  if(HR_STATE) fill(red);
  else fill(white);
  text(str(HeartRate), scrW/6, scrH/4 + scrH/5);
  text(str(Adrenaline), scrW/6, scrH/2 + scrH/8 + scrH/5);
  text(str(NeuroActivity), scrW - scrW/6, scrH/4 + scrH/5);
  text(str(Adrenaline), scrW - scrW/6, scrH/2 + scrH/8 + scrH/5);
}