import processing.serial.*;

byte myId;
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


  commands = new String[3];
  commands[0] = "p: Change serial port";
  commands[1] = "o: Send data to serial port";
  commands[2] = "ID: 0";

  frameRate(20);
}

void draw() {
  background(51);

  text("Current port: " + allPorts[port], 10, 50);
  for (int i = 0; i < commands.length; i++) {
    text(commands[i], 10, i*35+100);
  }
}


void keyPressed() {
  if (key >= '0' && key <= '7') { 
    myId = (byte)(key - '0');
    commands[2] = "ID: " + key;
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

  myPort.write((byte) 0xF5); // Lock byte 
  myPort.write(myId); 
  myPort.write((byte) 0x00); 
  myPort.write((byte) 0x00); 


  myPort.stop();
};