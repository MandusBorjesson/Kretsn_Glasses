 //<>// //<>// //<>//

class glasses { 
  int ypos, xpos, wdt, pixelW, numFrames, numImages; 
  int[] data, lookup;

  String[] sensors;
  byte currentSensor = 0;


  glasses (int xPos, int yPos, int xSize) {  
    ypos = yPos; 
    xpos = xPos; 
    wdt = xSize;

    pixelW = xSize/22; // 16 + 6 "dead" spaces
    numFrames  = 8;
    numImages  = 3;
    data = new int[numFrames*8];

    sensors = new String[8];
    sensors[0] = "Fast time";
    sensors[1] = "Medium time";
    sensors[2] = "Slow time";
    sensors[3] = "Mic volume";
    sensors[4] = "Accelerometer X";
    sensors[5] = "Accelerometer Y";
    sensors[6] = "Accelerometer Z";
    sensors[7] = "Static";
  } 

  int numFrames() {
    return numFrames;
  }

  int numSensors() {
    return sensors.length;
  }

  void setSensor(byte n){
    currentSensor = n;
  }

  String activeSensor() {
    return sensors[currentSensor];
  }

  void render(int offset, int yOffset) { 
    if (yOffset != 0) {
      fill(200);
      noStroke();
      rect(xpos-pixelW, yOffset+ypos-pixelW, wdt+2*pixelW, 10*pixelW);
    }

    stroke(51);
    fill(0);

    int bleed = pixelW/2;
    beginShape();
    vertex(xpos-bleed, yOffset + ypos-bleed); //Upper left
    vertex(xpos-bleed, yOffset + ypos+8*pixelW+bleed); //Lower left
    vertex(xpos+8*pixelW+bleed, yOffset + ypos+8*pixelW+bleed);  //Inner down left
    vertex(xpos+8*pixelW+bleed, yOffset + ypos+4*pixelW+bleed);  //Inner up left
    vertex(xpos+14*pixelW-bleed, yOffset + ypos+4*pixelW+bleed);  //Inner up right
    vertex(xpos+14*pixelW-bleed, yOffset + ypos+8*pixelW+bleed);  //Inner down right
    vertex(xpos+22*pixelW+bleed, yOffset + ypos+8*pixelW+bleed);  //Lower right
    vertex(xpos+22*pixelW+bleed, yOffset + ypos-bleed);  //Upper right
    endShape(CLOSE);

    noStroke();
    if (yOffset == 0) { 
      int index = (offset - 1 >= 0) ? offset - 1 : numFrames-1;

      drawPixels(index, 0, 70, 0, 0);
    }
    drawPixels(offset, yOffset, 200, 0, 0);
  }

  void drawPixels(int offset, int yOffset, int r, int g, int b) {
    for (int row = 0; row < 8; row++) {

      for (int col = 0; col < 8; col++) {
        if ((data[row+8*offset] & (0x8000>>col)) > 0) {
          fill(r, g, b);
        } else { 
          noFill();
        }

        rect(xpos + col*pixelW, yOffset + ypos+ row*pixelW, pixelW, pixelW);
      }

      for (int col = 0; col < 8; col++) {
        if ((data[row+8*offset] & (0x0080>>col)) > 0) {
          fill(r, g, b);
        } else { 
          noFill();
        }

        rect(xpos + 14*pixelW + col*pixelW, yOffset + ypos+ row*pixelW, pixelW, pixelW);
      }
    }
  }

  int isInside() {
    if (mouseX > xpos && mouseX < (xpos+8*pixelW) && mouseY > ypos && mouseY < ypos + 8*pixelW) {
      return 1;
    }

    if (mouseX > xpos + 14*pixelW && mouseX < (xpos+22*pixelW) && mouseY > ypos && mouseY < ypos + 8*pixelW) {
      return 2;
    }

    return 0;
  }

  void togglePix(int offset) {
    if (isInside() == 1) //Left eye
    {
      int col = floor( (mouseX - xpos) / pixelW );
      int row = floor( (mouseY - ypos) / pixelW );

      data[8*offset+row] ^= (0x8000>>col);
    }

    if (isInside() == 2)//Right eye
    {
      int col = floor( (mouseX - xpos) / pixelW )-6;
      int row = floor( (mouseY - ypos) / pixelW );

      data[8*offset+row] ^= (0x8000>>col);
    }
  }

  void fillPix(int offset) {
    if (isInside() == 1) //Left eye
    {
      int col = floor( (mouseX - xpos) / pixelW );
      int row = floor( (mouseY - ypos) / pixelW );

      data[8*offset+row] |= (0x8000>>col);
    }

    if (isInside() == 2)//Right eye
    {
      int col = floor( (mouseX - xpos) / pixelW )-6;
      int row = floor( (mouseY - ypos) / pixelW );

      data[8*offset+row] |= (0x8000>>col);
    }
  }

  void clearAll() {
    for (int i = 0; i < data.length; i++) {
      data[i]=0;
    }
  }

  void clearState(int state) {
    for (int i = state*8; i < state*8+8; i++) {
      data[i]=0;
    }
  }

  int isStatic() {
    for (int i = 8; i < data.length; i++) {
      if (data[i] != 0) return 0;
    }
    return 1;
  }

  char reverseBits(char x) {
    char b = 0;

    for (int i=0; i<8; i++) {
      b<<=1;
      b|=( x &0x01);
      x>>=1;
    }

    return b;
  }

  void writeImage(Serial thePort, char index) {
    thePort.write((byte) 0x02); // 0x02 => static image
    thePort.write((byte) index);// index
    thePort.write((byte) 0x00); // not used
    thePort.write((byte) 0x00); // not used

    for (int i = 0; i < 8; i++) {
      //println(binary(data[i+index*8], 16));
      thePort.write(reverseBits((char)(data[i + index*8] >> 8)));  //MSB
      thePort.write(reverseBits((char)(data[i + index*8] & 0xFF)));//LSB
    }
    //    println();
  }

  void writeToSerial(Serial thePort) {
    if (isStatic()==0) {
      thePort.write((byte) 0x01); // 0x01 => animated image
      for (char i = 0; i < numFrames; i++) {
        delay(400);
        writeImage(thePort, i);
      }
    } else {
      writeImage(thePort, (char)0);
    }
  }
}