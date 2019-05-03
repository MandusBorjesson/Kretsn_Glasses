import processing.serial.*;

byte txData;
PFont font;
int port;
String[] allPorts;
String[] commands;

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


  commands = new String[1];
  commands[0] = "p: Change serial port";
  
  txData = 0x00;
  frameRate(10);
}

void draw() {
  background(51); //<>//

  text("Current port: " + allPorts[port], 10, 50);
  for (int i = 0; i < commands.length; i++) {
    text(commands[i], 10, i*35+100);
  }
  
  for(int i = 0; i < 8; i++){ //<>//
    fill(((txData & (0x80>>i)) == 0) ? 0: 100);
    rect(i * width/8, height-width/8 , width/8,width/8);
  }
  fill(200); //<>//

  sendSerial(allPorts[port], 1200, txData);
  
//  delay(100);
  
  
}


void keyPressed() {
  if(key == 'z') txData |= 0x80;
  if(key == 'x') txData |= 0x40;
  if(key == 'c') txData |= 0x20;
  if(key == 'v') txData |= 0x10;
  if(key == 'b') txData |= 0x08;
  if(key == 'n') txData |= 0x04;
  if(key == 'm') txData |= 0x02;
  if(key == ',') txData |= 0x01;
  
    if (key == 'p') {
    port++;
    if (port >= allPorts.length) port = 0;
  }

}

void keyReleased() {
  if(key == 'z') txData &=~ 0x80;
  if(key == 'x') txData &=~ 0x40;
  if(key == 'c') txData &=~ 0x20;
  if(key == 'v') txData &=~ 0x10;
  if(key == 'b') txData &=~ 0x08;
  if(key == 'n') txData &=~ 0x04;
  if(key == 'm') txData &=~ 0x02;
  if(key == ',') txData &=~ 0x01;
}

void sendSerial(String thePort, int theBaud, byte Data) {
  if (thePort.equals("NO AVAILABLE PORTS.")) { //<>//
    println("No port found");
    return; //<>//
  }

  myPort = new Serial(this, thePort, theBaud); //<>//

  myPort.write((byte) 0xD3);  //<>//
  myPort.write((byte) Data); 
  
  myPort.stop(); //<>//
};