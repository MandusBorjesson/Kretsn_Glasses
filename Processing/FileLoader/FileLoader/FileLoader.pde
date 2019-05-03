String desktopPath = System.getProperty("user.home") + "/Desktop";
desktopPath = desktopPath.replace("\\", "/");

byte[] inBuf = new byte[131];
int[]  outBuf= new int[64];

inBuf = loadBytes(desktopPath + "/image.dat");

for(int i = 0; i < 64; i++){
  
  print(binary(inBuf[i*2]) + "      ");println(binary(inBuf[i*2+1]));
  if((i%8)==0){
    println();
  }
  
//  outBuf[i]   = (int)(((int)inBuf[i*2] << 8) | (int)inBuf[i*2+1]);      
}

/*
// Print each value, from 0 to 255 
for (int i = 0; i < outBuf.length; i++) { 
  // Every tenth number, start a new line 
  if((i%8)==0){
    println();
  }

  println(binary(outBuf[i])); 
  
}*/