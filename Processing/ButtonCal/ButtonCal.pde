import processing.serial.*;

byte myId;
PFont font;
int currBtn = 0;
int port;
String[] allPorts;
String[] commands;

PVector[] buttonPos;


Serial myPort;


void setup() {
  size(800, 300);
  port = 0;  

  if (Serial.list().length > 0) {
    allPorts = Serial.list();
  } else {
    allPorts = new String[1];
    allPorts[0] = "NO AVAILABLE PORTS.";
  }

  font = createFont("Comic Sans MS", 32);
  textFont(font);


  commands = new String[2];
  commands[0] = "p: Change serial port";
  commands[1] = "o: Calibrate button";

  buttonPos = new PVector[6];
  
  buttonPos[0] = new PVector(90,50);
  buttonPos[1] = new PVector(90,-50);
  buttonPos[2] = new PVector(-90,50);
  buttonPos[3] = new PVector(0,50);
  buttonPos[4] = new PVector(0,0);
  buttonPos[5] = new PVector(0,-50);

  frameRate(20);
}

void draw() {
  background(51);

  text("Current port: " + allPorts[port], 10, 50);
  for (int i = 0; i < commands.length; i++) {
    text(commands[i], 10, i*35+100);
  }

  pushMatrix();
  translate(width*0.75, height/2);
  drawController();
  popMatrix();
}


void keyPressed() {
  if (key >= '0' && key <= '5') { 
    currBtn = (byte)(key - '0');
  }

  if (key == 'p') {
    port++;
    if (port >= allPorts.length) port = 0;
  }
  if (key == 'o') sendSerial(allPorts[port], 1200);
}


void sendSerial(String thePort, int theBaud) {
  if (thePort.equals("NO AVAILABLE PORTS.")) {
    println("No port found");
    return;
  }

  myPort = new Serial(this, thePort, theBaud);

  myPort.write((byte) 0xA5); 
  print("Calibrating button: ");
  println(currBtn);
  myPort.write((byte) currBtn); 

  myPort.stop();
};


void drawController() {
  rectMode(CENTER);
  
  noStroke();

  fill(170);
  rect(0, 0, 180, 200);

  fill(0);
  for (int i = 0; i < 6; i++) {
    fill(i == currBtn? 255:0);
    
    rect(buttonPos[i].x, buttonPos[i].y, 40, 40);
  }
  
  fill(255);
};