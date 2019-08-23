

import processing.serial.*;

int IMG_MEM_BASE = 60928; //0xEE00; // Image data address
int IMG_MEM_SIZE = 4096;  // 0x1000; // Image data size, 256 frames
int DESC_MEM_BASE = 0x1080; // Configuration data address

PFont font;
int port;
int baud = 9600;
String[] allPorts;
String[] commands;
int nDescriptors = 8;
Serial myPort;
int offset = 0;
String modes[] = {"FRAME", "SWEEP UP", "SWEEP DOWN"};

public class Setting {
 public String name, type;
 public int period, offset, size, mode, eyes;

 Setting(String name) {
   this.name = name;
   //this.type = "FRAME";
   this.mode = 1;
   this.period = 1000;
   this.eyes = 0;
 }

 void display() {
   println("Path:   " + this.name);
   println("Mode:   " + modes[this.mode-1]);
   println("Period: " + str(this.period));
   println("Offset: " + hex(this.offset));
   println("Size:   " + str(this.size));
   println("Eyes:   " + String.format("%8s", Integer.toBinaryString(this.eyes)).replace(' ', '0') );

 }
}

//Setting[] settings;
Setting[] settings = new Setting[nDescriptors];
int settingcounter = 0;
int[] temp = new int[0];
byte[] data = new byte[0];
byte[] binaryGlassData = new byte[IMG_MEM_SIZE];

void setup() {
 size(800, 200);
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
     for (int j = 0; j < filenames.length; j++) {
       String file = filenames[j];

       String[] m = match(file, "^IMG"+str(i)+"[^.]*.png"); // Is there a file called IMGi--------.png?

       if(m != null) // Found file with current index
       {
         Setting setting = new Setting(file);

         m = match(file, "_M-?([1-"+str(modes.length)+"])"); // Look for mode switch
         if(m != null) setting.mode = int(m[1]);

         m = match(file, "_P-?(\\d+)"); // Look for Period switch
         if(m != null) setting.period = int(m[1]);

         m = match(file, "_L([0-7])"); // Look for left eye switch
         if(m != null) setting.eyes += Integer.parseInt(m[1])<<4;

         m = match(file, "_R([0-7])"); // Look for Right eye switch
         if(m != null) setting.eyes += Integer.parseInt(m[1]);

         // Add setting to array 
         settings[settingcounter++] = setting;
       }
     }
   }

   // Make image data for glasses from images
   offset = 0;
   for (int a = 0; a < settingcounter; a++) {
     println("======================");
     byte[] data = processimage(selection.getAbsolutePath() + "/" + settings[a].name);

     settings[a].offset = offset;
     settings[a].size = data.length;

     arrayCopy(data, 0, binaryGlassData, offset, data.length);
     offset += data.length;

     settings[a].display();
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
 int h = img.height;
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

   data[row*2]   = rEye;
   data[row*2+1] = lEye;
 }

 return data;
}

void writeToGlasses(byte[] data) {
 int theBaud = baud;
 String thePort = allPorts[port];

 if (thePort.equals("NO AVAILABLE PORTS.")) {
   println("No port found");
   return;
 }

 myPort = new Serial(this, thePort, theBaud);

 int base = IMG_MEM_BASE;
 byte[] chunked = new byte[64];

 print("Erasing image memory...");
 myPort.write(new byte[]{'e', 'r', (byte)0xEE, 0x00}); // Clear Images
 delay(400);
 println("Done");

 for (int i = 0; i < offset; i+=64) {
   print("Writing data to: " + hex(base+i) + "...");
   arrayCopy(data, i, chunked, 0, chunked.length);
   byte addrLSB = (byte) ((i+base)&0xFF);
   byte addrMSB = (byte) (((i+base)>>8)&0xFF);

   byte[] CMD = {'w', 'r', addrMSB, addrLSB, (byte)chunked.length, 0x01};

   myPort.write(concat(CMD, chunked));
   delay(400);
   println("Done");
 }

 print("Erasing descriptor memory...");
 myPort.write(new byte[]{'e', 'r', (byte)0x10, (byte)0x80}); // Clear descriptors
 delay(400);
 println("Done");

 for (int i = 0; i < chunked.length; i++) {
   chunked[i] = (byte)0xFF;
 }

 for (int i = 0; i < settingcounter; i++) {
   Setting s = settings[i];
   byte[] desc = new byte[8];
   desc[0] = (byte)(s.offset>>8);
   desc[1] = (byte)(s.offset&0xFF);
   desc[2] = (byte)(s.size>>8);
   desc[3] = (byte)(s.size&0xFF);
   desc[4] = (byte)(floor(s.period/100)&0xFF); 
   desc[5] = (byte)(s.mode&0xFF);
   desc[6] = (byte)(s.eyes);
   desc[7] = 0;
   arrayCopy(desc, 0, chunked, i*desc.length, desc.length);
 }

 print("Writing data to descriptor memory...");
 byte addrLSB = (byte) ((DESC_MEM_BASE)&0xFF);
 byte addrMSB = (byte) (((DESC_MEM_BASE)>>8)&0xFF);

 byte[] CMD = {'w', 'r', addrMSB, addrLSB, (byte)chunked.length, 0x01};
 myPort.write(concat(CMD, chunked));
 delay(400);
 println("Done");
 println();
 println("Wrote following descriptors at address: 0x" + hex(CMD[2]) + hex(CMD[3]) + ":");
 println();
 println("                / Period");
 println("/ Offset        |   / Mode");
 println("|       / Size  |   |   / Eyes  ");
 println("|_____  |_____  |_  |_  |_");
 for(int i = 0; i < chunked.length; i++){
   print(hex(chunked[i]) + ", ");
   if((i+1)%8 == 0)
     println();
 }

 myPort.stop();
}