import processing.serial.*;
import ddf.minim.*;
Minim minim;
AudioPlayer finalMusic;
Serial arduino;
String portName;
PImage background;
int HeartRate, NeuroActivity, Adrenaline, Oxygen;
int newHeartRate;
float scrH, scrW;
boolean game;
boolean connected;
boolean win, winStart, panic, showPanicText, Done;
int blue, red, white;
int panicCount;
long now;
long panicDelay, shutDownDelay;
void setup()
{
  //Draw settings//
  background(0);
  //fullScreen();
  size(1000, 600);
  scrW = width;
  scrH = height;
  background = loadImage("back.jpg");
  blue = color(0, 255, 255);
  red = color(253, 0, 26);
  white = color(253, 254, 255);
  println("Connecting...");
  //Arduino connection manager//
  arduinoConnect();
  println("connected");
  win = false;
  winStart = false;
  panic = true;
  showPanicText = true;
  Done = false;
  HeartRate = 100;
  NeuroActivity = 100;
  Adrenaline = 100;
  Oxygen = 80;

  shutDownDelay = 2000;
  panicDelay = 1500;
  panicCount = 0;
  minim = new Minim(this);
  finalMusic = minim.loadFile("final.wav");
}

void draw()
{
  if (win)
  {
    background(0);
    if (panic) showPanic();
    else
    {
      if (millis() - now < shutDownDelay) showShutDown();
      else noLoop();
      println("shutdown");
    }
  } else
  {
    showBackground();
    showHeaders();
    if(winStart)
    {
      if(millis() - now > 1000 && Done) 
      {
        win = true;
        now = millis();
        finalMusic.play();
      }
    }
    getUpdates();
    showValues();
  }
}

void arduinoConnect()
{
  connected = false;
  String[] comportConfig = loadStrings("com.txt");
  if (comportConfig.length > 0) portName = comportConfig[0].substring(comportConfig[0].indexOf("\"") + 1, comportConfig[0].length() - 2);
  arduino = new Serial(this, portName, 9600);
  long startConnect = millis();
  while (!connected || millis() - startConnect < 10000)
  {
    arduino.write("letsgame\n"); 
    long now = millis();
    while (millis() - now < 1000) {
      ;
    }
    println("try...");
    String input = getInput(); 
    if (input.length() > 4)
    {
      if (input.substring(0, 5).equals("start")) connected = true;
    }
  }
  if (!connected) { 
    noLoop(); 
    println("No connection for arduino..."); 
    text("NO ARDUINO CONNECTED TO PC", width/2, height/2);
  }
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
  if (inData.length() > 3)
  {
    if (inData.substring(0, 3).equals("END"))
    {
      Done = true;
      now = millis();
    } else
    {
      println(inData);
      int[] Data = int(inData.split(";"));
      if (Data.length > 3)
      {
        newHeartRate = Data[0];
        NeuroActivity = Data[1];
        Adrenaline = Data[2];
        Oxygen = Data[3];
      }
    }
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
  if(HeartRate > newHeartRate) HeartRate--;
  if(HeartRate < newHeartRate) HeartRate++;
  if (HeartRate == 240) { fill(red); winStart = true;}
  else fill(white);
  text(str(HeartRate), scrW/6, scrH/4 + scrH/5);
  fill(white);
  text(str(Adrenaline), scrW/6, scrH/2 + scrH/8 + scrH/5);
  text(str(NeuroActivity), scrW - scrW/6, scrH/4 + scrH/5);
  text(str(Oxygen), scrW - scrW/6, scrH/2 + scrH/8 + scrH/5);
}

void showPanic()
{
  textSize(30);
  fill(blue);
  textAlign(CENTER);
  text("NEURODOPE Health Monitor HM50021", scrW/2, scrH/16);
  if (showPanicText) 
  {
    fill(255, 0, 20);
    textAlign(CENTER);
    textSize(50);
    text("System is disabled!", scrW/2, scrH/3);
    text("**Cut-off mode**", scrW/2, 2*scrH/3);
    println("panic");
  }
  if (millis() - now > panicDelay) 
  { 
    
    showPanicText = !showPanicText; 
    now = millis();
    panicCount++;
    if (panicCount == 4) panic = false;
  }
}

void showShutDown()
{
  fill(255, 0, 20);
  textAlign(CENTER);
  textSize(50);
  text("SYSTEM SHUTDOWN!", scrW/2, scrH/2);
}