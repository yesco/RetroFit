// Simple ANIM graphics.


// Usage: #include "anim.c" LOL

// TOOD: make independent?
// User extensible scripts!

void drawX() {
  for(int rr=1; rr<=screen_rows-2; rr++) {
    //B(white);
    //gotorc(rr, 0); clearend();
    int col= screen_cols*rr/screen_rows;
    gotorc(rr, col-1); B(black); spaces(3);
    gotorc(rr, screen_cols-col-1); B(black); spaces(3);
  }
  usleep(10*1000);
}

void drawPullDownMenu(color *colors, char **labels, int n) {
  int cc= 40*screen_cols/100;
  int rows= 0;
  while (rows+3 < screen_rows) {
    rows++;
    // exponential decrease of slide
    usleep(70*SQR(rows)/SQR(screen_rows)/2*1000);

    for(int rr=0; rr<rows; rr++) {
      gotorc(rr, screen_cols-cc);
      color col= colors[n*rr/rows];
      B(col);
      spaces(cc);
      char* str= labels[n*rr/rows];

      // center label
      int w= (cc-strlen(str))/2;
      gotorc(rr, screen_cols-cc + w);
      readablefg();
      printf("%s", str);
    }
    while(haskey()) key();
  }
}

