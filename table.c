// table.c part of w.c minimal webbrowser
// (>) CC-BY 2021 Jonas S Karlsson

// Status:
//   - being worked on, exploratory
//   - bad hack!
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
// WARNING: table handling and rendering
//          is just a hack for now, bad
//          explorative hack!
// TODO: need a stack? <a> inside <td>?

// REFERENCES:
// W3C suggested layout (not used here)
// - https://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#h-B.5.2.2

typedef struct tcol {
  int i, span;
  bool head;
  char* s;
  int w, h, len; // in chars
  char align; // l(eft) r(ight) c(enter) '.' for decimal
  char* tag; // TODO: remove
} tcol;

dstr* content = NULL;
dstr *table = NULL;
int tdn= 0, tdi= 1, ty=0, tx=0, tp= 0;

#define TD_MAX 1000
tcol tds[TD_MAX] = {0};
int incell= 0;

void handle_trd(int isRow, int isEndTag, TAG tag) {
  static bool head= false;
  assert(tdn < TD_MAX);

  // end
  if (incell) {
    // store previous cells info
    // TODO: should set indent=0 ???
    tcol* t= &tds[tdn++];
    t->i= tdi;
    t->span= 1;
    t->s= strdup(table->s+tp);
    t->len= (_cury-ty)*cols + _curx-tx;
    t->w= strlen(t->s); // < as \n
    t->h= _cury-ty;;
    t->tag= strdup(tag);
    t->head= head;
    //B(green); printf("[%s]", t->tag); B(white);

    tdi++;
    incell= 0; head= false;
  }

  if (strstr(" td th ", tag)) incell= 1;
  head= strstr(" th ", tag);

  //C(blue); printf("[%s]", tag); C(black);

  // ok, now we're ready for new element
  ty= _cury; tx= _curx;
  tp= strlen(table->s);

  if (isRow) {
    // insert a zero elemetn for each new line
    memset(&tds[tdn++], 0, sizeof(tcol));
    tdi= 1;
  }
}

void renderTable() {
  C(green); B(black);
  printf("\n----------TABLE----------\n");
  //printf("%s\n<<<---STRING\n", table->s);
#define MAX_COLS 11
  int w[MAX_COLS]= {0}, h[MAX_COLS]= {0};
  int sum_w[MAX_COLS]= {0}, sum_h[MAX_COLS]= {0};
  int rows= 0;

  // print data
  for(int i=0; i<tdn; i++) {
    tcol* t= &tds[i];

    if (!t->i) rows++;
 
    { int a= w[t->i], b= t->w;
      sum_w[t->i]+= b;
      w[t->i] = a > b ? a: b;
    }
 
    { int a= h[t->i], b= t->h;
      sum_h[t->i]+= b;
      h[t->i] = a > b ? a: b;
    }
 
    printf("===%2d: i=%1d span=%1d h=%2d w=%2d l=%3d '%s' ... tag=%s\n",
           i, t->i, t->span, t->h, t->w, t->len,t->s?t->s:"(NULL)", t->tag);
  }

  // print stats
  printf("\nROWS=%d\n", rows);
  printf("\niiii::::: ");
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ", i);
  printf("\nWIDTH::: ");
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ", w[i]);

  int a[MAX_COLS]= {0};
  for(int i= 0; i<MAX_COLS; i++)
    a[i]= (sum_w[i]+rows/2)/rows;

  printf("\n   AVG:: ");
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ",  a[i]);

  printf("\n  A.SUM: "); int width= 0;
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ",  width+=(sum_w[i]+rows/2)/rows);

  printf("\n\nHEIGHT:: ");
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ", h[i]);
  printf("\n   AVG:: ");
  for(int i= 0; i<MAX_COLS; i++) printf("%3d ", (sum_h[i]+rows/2)/rows);
  printf("\n---END\n", table->s);
  C(black); B(white);

  // init format
  // TODO: make it max of td in column
  //   keep td width separate
  int f[MAX_COLS]= {0};
  for(int i= 0; i<MAX_COLS; i++) {
    int v= a[i];
    if (!v) continue;
    f[i]= v < 3 ? 3 : v;
    int d= (sum_w[i] - tds[i].w) / rows;
    printf("=== %d: %d %d\n", i, f[i], d);
  }

  // TODO: if have more space allocate
  // according to "bigger" (title?)
  //
  // shrink, starting with biggest columns
  // TODO: distribute more evenly, maybe
  //   remove 2 from biggest, 1 from next
  int overflows= 0;

  //const char* sbaseletter="𝗮";
  const char* sbaseletter="₀";
  //const char* sbaseletter="ₐ";
  wchar_t baseletter;
  size_t dummyl;
  u8_to_u32(sbaseletter, strlen(sbaseletter), &baseletter, &dummyl);
  assert(dummyl==1);
  //printf("-------baseletter=%x\n", baseletter);

  wchar_t letter[MAX_COLS]= {0};
  
  // init total and tcolums
  int total= 0, tcolumns= 0;
  for(int i=0; i<MAX_COLS; i++) {
    total+= f[i];
    if (f[i]) tcolumns++;
    //printf("--%d %d\n", i, f[i]);
  }

  while (total>cols-tcolumns) {
    printf("\n--------MINIMIZE------ %d %d %d\n", total, cols, tcolumns);
    // find biggest
    int big= -1, max= 0;
    total= 0;
    for(int i=0; i<MAX_COLS; i++) {
      int v= f[i];
      total+= v;
      if (v>max) max= v, big= i;
    }
    // decrease allocation
    if (big < 0) break; // screwed
    f[big]--;
  }

  // actually render
  int row= 0;
  for(int i=0; i<tdn; i++) {
    tcol* t= &tds[i];

    if (!t->i) putchar('\n');
 
    char* s= t->s;
    if (s) {
      if (t->head) underline();

      for(int j=f[t->i]; j>0; j--) {
        //if (*s && *s!=10 && (*s<32 || *s>128)) printf("[%d]", *s);
        if (!*s) putchar(' ');
        if (!*s && t->head) end_underline();
        else if (*s == 10) ;
        else if (*s == 13) ;
        else if (*s == (char)HNL) ;
        else if (*s == (char)SNL) printf("↲");

        else if (*s == '\n') {
          do s++; while (*(s+1) && (*s==(char)HNL || *s==(char)SNL));
          printf("↲");
        }
        else if (*s<32 || *s>127) ; // TODO:
        else putchar(*s);

          if (*s) s++;
        }
        if (t->head) end_underline();

        // overflow?
        if (!*s) { // not overflow
          putchar(' ');
        } else {
          if (!t->head) {// <TD>
            printf("↲");
            // TODL: save s for next overflow line! print
          } else {
            overflows++;
            letter[t->i]= baseletter+overflows-1;
            putwchar(letter[t->i]);
          } C(fg); B(bg);
        }
    }
    row++;
  }

  printf("\n");
  if (overflows) {
    underline();
    printf("\nLegends");
    end_underline();
    nl();
  }
  // print overflow data
  for(int i=0; i<MAX_COLS; i++) {
    wchar_t l= letter[i];
    if (!l) continue;
    printf("  ");
    putwchar(l); //putchar(' ');
    // hmmm td[i] - lots off assumption
    tcol* t= &tds[i];
    assert(t->i==i); // it's header?
    // TODO: make a function of printer above...
    printf("%s", t->s);
  }
  if (overflows) printf("\n");

  // cleanup
  free(table); table= NULL;

  tdn= 0, tdi= 1, ty=0, tx=0, tp= 0;
  incell= 0;
  memset(tds, 0, sizeof(tds));
}
