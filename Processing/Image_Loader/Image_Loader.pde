import processing.serial.*;

int IMG_MEM_BASE = 60928; //0xEE00; // Image data address
int IMG_MEM_SIZE = 4096;  // 0x1000; // Image data size, 256 frames
int DESC_MEM_BASE = 0x1080; // Configuration data address

PFont font;
int port;
int baud = 1200;
String[] allPorts;
String[] commands;
int nDescriptors = 8;
Serial myPort;
int offset = 0;

int str2mode(String str) {
  if(str.equals("FRAME"))
    return 1;
  if(str.equals("SWEEP"))
    return 2;
    
  return -1;
}


public class Setting {
  public String name, type;
  public int period, offset, frames, mode;

  Setting(String name) {
    this.name = name;
    this.type = "FRAME";
    this.mode = 0;
    this.period = 1000;
  }

  Setting(String name, String type, int period) {
    this.name = name;
    this.type = type;
    this.mode = str2mode(type);
    this.period = period;
  }

  void display() {
    println("Path:   " + this.name);
    println("Type:   " + this.type);
    println("Period: " + str(this.period));
    println("Offset: " + hex(this.offset));
    println("Frames: " + str(this.frames));
  }
}

//Setting[] settings;
Setting[] settings = new Setting[nDescriptors];
int settingcounter = 0;
int[] temp = new int[0];
byte[] data = new byte[0];
byte[] binaryGlassData = new byte[IMG_MEM_SIZE];

void setup() {
  size(1200, 800);
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
  commands[1] = "l: Load image files";
  commands[2] = "Space: Send data to serial port";

  frameRate(10);
}


void draw() {
  background(51);

  text("Current port: " + allPorts[port], 10, 50);
  for (int i = 0; i < commands.length; i++) {
    text(commands[i], 10, i*35+100);
  }
}

void keyPressed() {
  if (key == 'p') {
    port++;
    if (port >= allPorts.length) port = 0;
  }
  if (key == 'l') {
    loadImages();
  }
  if (key == ' ') {
    writeToGlasses(binaryGlassData);
  }
}


void sendSerial(String thePort, int theBaud) {
  if (thePort.equals("NO AVAILABLE PORTS.")) {
    println("No port found");
    return;
  }

  myPort = new Serial(this, thePort, theBaud);

  myPort.stop();
};

void loadImages() {
  selectFolder("Select a folder with images in:", "folderSelected");
}


void folderSelected(File selection) {
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    println("User selected " + selection.getAbsolutePath());
    settingcounter = 0;
    // we'll have a look in the data folder
    java.io.File folder = new java.io.File(selection.getAbsolutePath());

    // list the files in the data folder
    String[] filenames = folder.list();

    // Attempt to find images
    for (int i = 0; i < nDescriptors; i++) {
      String multiArg = "IMG" + (str(i)) + ("_");
      String singleArg = "IMG" + (str(i)) + (".");
      for (int j = 0; j < filenames.length; j++) {
        String file = filenames[j];

        if (file.indexOf(singleArg) == 0)
        {
          Setting setting = new Setting(file);
          settings[settingcounter++] = setting;
          break;
        } else if (file.indexOf(multiArg) == 0)
        {
          String[] temp = split(split(file, ".")[0], "_");
          Setting setting = new Setting(file, temp[1], abs(int(temp[2])));
          settings[settingcounter++] = setting;
          break;
        }
      }
    }

    // Make image data for glasses from images
    offset = 0;
    for (Setting a : settings) {
      println("======================");
      byte[] data = processimage(selection.getAbsolutePath() + "\\" + a.name);

      a.offset = offset;
      a.frames = floor(data.length/16);

      arrayCopy(data, 0, binaryGlassData, offset, data.length);
      offset += data.length;

      a.display();
    }
    println("Total consumed area: " + offset + " bytes, " + str(100*offset/IMG_MEM_SIZE) + "%");
    if (offset > IMG_MEM_SIZE) {
      print("ERROR: Images occupy more than available space!!");
    }
  }
}

byte[] processimage(String path) {
  PImage img = loadImage(path);
  img.loadPixels();
  int w = img.width;
  int h = img.height - img.height%8; // Remove any pixels that dont add up to an even frame number
  int[] temp = new int[w];
  byte[] data = new byte[h*2];


  for (int row = 0; row < h; row++) {
    arrayCopy(img.pixels, w*row, temp, 0, w);

    for (int a = 0; a < w; a++) {
      temp[a] = ((temp[a]&0xFF000000) == 0xFF000000) ? 0x01:0x00;
    }

    byte lEye = 0;
    byte rEye = 0; 

    for (int a = 0; a < 8; a++) {
      lEye |= temp[a] << (a);
      rEye |= temp[w-8+a] << (a);
    }

    data[row*2]   = lEye;
    data[row*2+1] = rEye;
  }

  return data;
}

void writeToGlasses(byte[] data) {
  int theBaud = baud;
  println(data.length);
  String thePort = allPorts[port];

  if (thePort.equals("NO AVAILABLE PORTS.")) {
    println("No port found");
    return;
  }

  myPort = new Serial(this, thePort, theBaud);

  byte[] writeCmd = {'w', 'r'};
  int base = IMG_MEM_BASE;
  byte[] chunked = new byte[64];

  println("Writing data to glasses...");
  print("Clearing flash memory...");

  myPort.write("cl"); // Clear Images
  delay(100);
  println("Done");

  for (int i = 0; i < offset; i+=64) {
    print("Writing data to: " + hex(base+i) + "...");
    arrayCopy(data, i, chunked, 0, chunked.length);
    byte addrLSB = (byte) ((i+base)&0xFF);
    byte addrMSB = (byte) (((i+base)>>8)&0xFF);

    byte[] CMD = {'w', 'r', addrMSB, addrLSB, (byte)chunked.length, 0x01};

    myPort.write(concat(CMD, chunked));
    delay(100);
    println("Done");
  }

  print("Clearing descriptor memory...");

  myPort.write("de"); // Clear descriptors
  delay(100);
  println("Done");

  for (int i = 0; i < chunked.length; i++) {
    chunked[i] = (byte)0xFF;
  }
  
  for (int i = 0; i < settingcounter; i++) {
    Setting s = settings[i];
    byte[] desc = new byte[6];
    desc[0] = (byte)(s.offset>>8);
    desc[1] = (byte)(s.offset&0xFF);
    desc[2] = (byte)(s.frames>>8);
    desc[3] = (byte)(s.frames&0xFF);
    desc[4] = (byte)(floor(s.period/100)&0xFF); 
    desc[5] = (byte)(s.mode&0xFF);
    println(hex(desc[5]));

    arrayCopy(desc, 0, chunked, i*desc.length, desc.length);
  }

  print("Writing data to descriptor memory...");
  byte addrLSB = (byte) ((DESC_MEM_BASE)&0xFF);
  byte addrMSB = (byte) (((DESC_MEM_BASE)>>8)&0xFF);

  byte[] CMD = {'w', 'r', addrMSB, addrLSB, (byte)chunked.length, 0x01};
  myPort.write(concat(CMD, chunked));
  delay(100);
  println("Done");

  myPort.stop();
}