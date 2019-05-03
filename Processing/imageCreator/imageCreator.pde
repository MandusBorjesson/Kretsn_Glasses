import processing.serial.*; //<>//

glasses glass1;
frameButton button;
frameButton button2;
byte lColor;
byte rColor;
PFont font;
int port;
int baud = 1200;
String[] allPorts;
String[] commands;

Serial myPort;


void setup() {
  size(1200, 800);
  port = 0;  
  
  lColor = 0;
  rColor = 0;

  if (Serial.list().length > 0) {
    allPorts = Serial.list();
  } else {
    allPorts = new String[1];
    allPorts[0] = "NO AVAILABLE PORTS.";
  }

  font = createFont("Comic Sans MS", 32);
  textFont(font);

  glass1 = new glasses((int)(width*0.4), height/2+50, 600);
  button = new frameButton(20, height-100, 60, glass1.numFrames(), "Frame number");
  button2 = new frameButton(20, height-200, 60, glass1.numImages, "");


  commands = new String[10];
  commands[0] = "c: Clear all frames";
  commands[1] = "f: Clear single frame";
  commands[2] = "x: Step forward";
  commands[3] = "z: Step backward";
  commands[4] = "p: Change serial port";
  commands[5] = "o: Flash current image";
  commands[6] = "Mouse click: toggle pixel";
  commands[7] = "Mouse drag: fill pixels";
  commands[8] = "Space: Send data to serial port";
  commands[9] = "s: Save image to desktop";



  frameRate(20);
}


void draw() {
  background(51);

  text("Current port: " + allPorts[port], 10, 50);
  for (int i = 0; i < commands.length; i++) {
    text(commands[i], 10, i*35+100);
  }

  
  if(glass1.isStatic() > 0){glass1.setSensor((byte)7);}
  text("Image animates with \n[use number keys]: \n    " + glass1.activeSensor(), 10, 470);


  glass1.render(button.state(), 0);
  glass1.render((glass1.isStatic() >0)? 0 : (millis()/300)%(glass1.numFrames()), -350);

  
  button2.meaning = (glass1.isStatic() > 0)? "Static image number": "Dynamic image number";
  button.render();
  button2.render();
  
  fill(255*(lColor&0x01),255*(lColor&0x02)>>1,255*(lColor&0x04)>>2); 
  stroke(255);
  ellipse(950,200,30,30);

  fill(255*(rColor&0x01),255*(rColor&0x02)>>1,255*(rColor&0x04)>>2); 
  stroke(255);
  ellipse(600,200,30,30);

  fill(255);
}

void mousePressed() {
  glass1.togglePix(button.state());

  if (button.isInside()>0) button.increaseState();
  if (button2.isInside()>0) button2.increaseState();
}

void mouseDragged() {
  glass1.fillPix(button.state());
}

void keyPressed() {
  if (key == 'c') glass1.clearAll();
  if (key == 'f') glass1.clearState(button.state());
  if (key == 'x') button.increaseState();
  if (key == 'z') button.decreaseState();
  if (key == 's') {
    String desktopPath = System.getProperty("user.home") + "/Desktop";
    desktopPath = desktopPath.replace("\\", "/");
    
    byte[] outBuf = new byte[131];
    
    for(int i = 0; i < 64; i++){
      outBuf[i]   = (byte)(glass1.data[i] >> 8);     // High byte 
      outBuf[i+1] = (byte)(glass1.data[i] &  0xFF);  // Low  byte
    }
    
    outBuf[128] = (byte)glass1.isStatic();
    outBuf[129] = (byte)((lColor << 4) | rColor);
    outBuf[130] = (byte)glass1.currentSensor;

    saveBytes(desktopPath + "/image.dat", outBuf);
  }
  if (key == 'p') {
    port++;
    if (port >= allPorts.length) port = 0;
  }
  if (key == ' ') sendSerial(allPorts[port], baud);

  if (key >= '0' && key <= glass1.numSensors() - 2 + '0') { 
    glass1.currentSensor = (byte)(key - '0');
  }

  if (key == 'o') {
    flashImage(glass1.isStatic(), button2.state(), glass1.currentSensor, allPorts[port], baud);
  }
  
  if(key == 'l'){
    lColor = (byte)((lColor+1)&0x07);
    println(str(lColor));
  }
  if(key == 'r'){
    rColor = (byte)((rColor+1)&0x07);
  }
  
}


void sendSerial(String thePort, int theBaud) {
  if (thePort.equals("NO AVAILABLE PORTS.")) {
    println("No port found");
    return;
  }

  myPort = new Serial(this, thePort, theBaud);

  glass1.writeToSerial(myPort);

  myPort.stop();
};

void flashImage(int isStatic, int number, byte dependence,String thePort, int theBaud) {
  if (thePort.equals("NO AVAILABLE PORTS.")) {
    println("No port found");
    return;
  }
  
  myPort = new Serial(this, thePort, theBaud);
  myPort.write((byte) (0xAD));     // check byte
  myPort.write((byte) ((lColor << 4) | (rColor & 0x0F))); 
  myPort.write((byte) (dependence)); 
  myPort.write((byte) ((isStatic << 4) | (number & 0x0F))); 
  myPort.stop();
};