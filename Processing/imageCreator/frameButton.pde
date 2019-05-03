class frameButton { 
  int xPos, yPos, size, state, max;
  String meaning;

  frameButton(int x, int y, int s, int theMax, String purpose) {
    xPos = x;
    yPos = y;
    size = s;
    max = theMax;
    state = 0;
    meaning = purpose;
  }

  void render() {

    fill(0);
    stroke(170);
    strokeWeight(4);

    rect(xPos, yPos, size, size); //Outer frame

    fill(100);
    rect(xPos, yPos + size, size, -(size*state)/(max-1)); //inner bar
     //<>//
    fill(255);
    textSize(size*0.4);
    text(state, xPos+size*0.1,yPos+size*0.8);
    text(meaning, xPos+size*1.1, yPos+size*0.8);
    
  }

  int state() {
    return state;
  }

  int isInside() {

    if (mouseX > xPos && mouseX < xPos + size && mouseY > yPos && mouseY < yPos + size) {
      return 1;
    }

    return 0;
  }

  void increaseState() {
    state++;
    if (state >= max) state = 0;
  }

  void decreaseState() {
    state--;
    if (state < 0) state = max -1;
  }
}