// File with experimenal UI-code
// (directly included by wless.c)
//
// once finalized, cleanedup etc,
// then move to wless.c


// future is +1, past is -1
keycode panHistory(keycode k, int future) {
  cursoroff();
  // TODO: merge with gohistory()
  //system("perl whi2html.pl > .whistory.html ; ./w.x .whistory.html > .whistory.html.ANSI");
  //FILE *fansi= fopen(file, "r");
  //FILE *f= fopen(".whistory.html.ANSI", "r");
  //FAILIF(!f || !fansi, "Can't view history");
  int top0= top, n= future;
  while (n && (k= key()) & SCROLL) {
    if (k==CTRL+'C') exit(0);

    // scroll 
    int kr= (k>>8) & 0xff, kc= k & 0xff;
    if (((kr<screen_rows/2)?+1:-1)==future) {
      tab+= k & SCROLL_UP? -1 : +1;
      tab= MAX(-start_tab, MIN(ntab-1, tab));
    } else {
      n+= k & SCROLL_UP? +1 : -1;
    }

    int tab1= tab;
    if (future==+1) {
      //n= MIN(ntab+1, MAX(0, n));
      n= MIN(rows/2, MAX(0, n));
      gotorc(1, 0);
      //printAnsiLines(fansi, top0, rows-n);
      loadPageMetaData();
      displayWin(k, url, file, top, rows-n+1, 0);
      cursoroff();
      gotorc(rows-n+1, 0);
      spaces((screen_cols-11)/2);
      B(black); C(orange);
      printf("F U T U R E"); clearend();

      // -- The Future
      //printAnsiLines(f, 0, n);
      for(int i=0; i<n-1; i++) {
        putchar('\n');
        if (i==tab) {
          B(purple); C(brightorange);
        } else {
          B(black); C(orange);
        }

        tab= i;
        loadPageMetaData();

        char *title= getTitle(url, file);
        char status= 'U';

        printf("%c%3d  %.*s", status, i, screen_cols-7, title); clearend();
        free(title);
      }
      B(black); C(white);
      fflush(stdout);

    } else { // -1 The Past
      //n= MIN(0, MAX(-start_tab, n));
      n= MIN(0, MAX(-rows/2, n));

      gotorc(0, 0);
      for(int i=0; i<-n; i++) {
        int t= i+n+1;
        if (t==tab) {
          B(purple); C(brightorange);
        } else {
          B(black); C(orange);
        }

        tab= t; loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'R';

        printf("%c%3d  %.*s", status, t, screen_cols-7, title); clearend(); putchar('\n');
        free(title);

      }
      B(black); C(orange);
      spaces((screen_cols-7)/2);
      printf("P A S T"); clearend();
      putchar('\n');
      
      gotorc(n, 0);

      tab= tab1;
      loadPageMetaData();
      FILE *fansi= fopen(file, "r");
      if (fansi) {
        printAnsiLines(fansi, top0, rows+n);
        fclose(fansi);
      }
      // TODO: starts from rc=0,0
      //displayWin(k, url, file, top, rows+n+1, 0);
      cursoroff();
    }
    tab = tab1;

    cleareos();
    fflush(stdout);
  }
  //fclose(f);
  //fclose(fansi);
  cursoron();
  while(haskey() && ((k= key()) & SCROLL));
  //return k & SCROLL? REDRAW : k;
  return REDRAW;
}

keycode newPanHistory(keycode k, int future) {
  cursoroff();
  // TODO: merge with gohistory()
  //system("perl whi2html.pl > .whistory.html ; ./w.x .whistory.html > .whistory.html.ANSI");
  //FILE *fansi= fopen(file, "r");
  //FILE *f= fopen(".whistory.html.ANSI", "r");
  //FAILIF(!f || !fansi, "Can't view history");
  int top0= top, n= future;
  while ((k= key()) & SCROLL) {
    if (k==CTRL+'C') exit(0);

    // scroll 
    int kr= (k>>8) & 0xff, kc= k & 0xff;
    //if (((kr<screen_rows/2)?+1:-1)==future) {
    //tab+= k & SCROLL_UP? -1 : +1;
    //tab= MAX(-start_tab, MIN(ntab-1, tab));
    //} else {
    const int step=5;
    n+= k & SCROLL_UP? +step : -step;
    //}

    int tab1= tab;
    if (n>=0) {
      //n= MIN(ntab+1, MAX(0, n));
      n= MIN(rows/2, MAX(0, n));
      gotorc(1, 0);
      //printAnsiLines(fansi, top0, rows-n);
      loadPageMetaData();
      displayWin(k, url, file, top, rows-n, 0);
      cursoroff();
      gotorc(rows-n+1, 0);

      // -- The Future
      //printAnsiLines(f, 0, n);
      for(int i=0; i<n; i++) {
        putchar('\n');
        B(red); C(white);
        tab= i;
        loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'U';

        printf("%c%3d  %.*s", status, i, screen_cols-7, title); clearend();
        free(title);
      }
      B(black); C(white);
      fflush(stdout);

    } else { // -1 The Past
      //n= MIN(0, MAX(-start_tab, n));
      if (n < -rows*2/3) tab1--;
      n= MIN(0, MAX(-rows*2/3, n));

      gotorc(0, 0);
      if (0) {
        int lns= 7;

        int i= 0;
        while(i<lns) {
          tab= tab1 + n/step - 1 + i;
          loadPageMetaData();
          char *title= getTitle(url, file);
          char status= 'R';
          B(black); C(orange);
          printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
          free(title);

          i++;
        }
        while(i<-n) {
          tab= tab1 + (i+n)/step - 1;
          loadPageMetaData();
          char *title= getTitle(url, file);
          char status= 'R';
          B(black); C(orange);
          printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
          free(title);

          FILE *fansi= fopen(file, "r");
          if (fansi) {
            printAnsiLines(fansi, top0, lns);
            fclose(fansi);
          }
          i+= lns;

          i++;
        }

      } else
      for(int i=0; i<-n; i++) {
        tab= tab1 + n/step + 1 + i;
        loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'R';
        B(black); C(orange);
        printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
        free(title);

      }
      gotorc(n, 0);
      clearend(); printf("\n");

      tab= tab1;
      loadPageMetaData();
      FILE *fansi= fopen(file, "r");
      if (fansi) {
        printAnsiLines(fansi, top0, rows+n+1);
        fclose(fansi);
      }
      // TODO: starts from rc=0,0
      //displayWin(k, url, file, top, rows+n+1, 0);
      cursoroff();
    }
    tab = tab1;

    cleareos();
    fflush(stdout);
  }
  //fclose(f);
  //fclose(fansi);
  cursoron();
  while(haskey() && ((k= key()) & SCROLL));
  //return k & SCROLL? REDRAW : k;
  return REDRAW;
}

