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
// First RFC (1996, old!)
// - https://www.ietf.org/rfc/rfc1942.txt
// W3C suggested layout (not used here)
// - https://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#h-B.5.2.2
// CSS table-layout property
// - https://www.w3schools.com/cssref/pr_tab_table-layout.asp
// CSS 2021 spec for tables
// - https://html.spec.whatwg.org/multipage/rendering.html#tables-2

typedef struct tcol {
  int i, span;
  int head;
  char* s;
  int w, h, len; // in chars
  char align; // l(eft) r(ight) c(enter) '.' for decimal
  char* tag; // TODO: remove
} tcol;

dstr* content = NULL;
dstr *table = NULL;
int tdn= 0, tdi= 1, ty=0, tx=0, tp= 0;

#define TD_MAX 3000
tcol tds[TD_MAX] = {0};
int incell= 0;

void handle_trd(int isRow, int isEndTag, TAG tag) {
  if (!table || !table->s) return;

  static int head= 0;
  // TODO: fix this...
  assert(tdn < TD_MAX);

  // end
  if (incell) {
    // store previous cells info
    // TODO: should set indent=0 ???
    tcol* t= &tds[tdn++];
    t->i= tdi;
    t->span= 1;
    t->s= strdup(table->s+tp);
    t->len= (_cury-ty)*screen_cols + _curx-tx;
    t->w= strlen(t->s); // < as \n
    t->h= _cury-ty;;
    t->tag= strdup(tag);
    t->head= head;
    //B(green); printf("[%s]", t->tag); B(white);

    tdi++;
    incell= 0; head= 0;
  }

  if (strstr(" td th ", tag)) incell= 1;
  head= !!strstr(" th ", tag);

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
  TRACE("\n----------TABLE----------\n");
  //TRACE("%s\n<<<---STRING\n", table->s);
#define MAX_COLS 11
  int w[MAX_COLS]= {0}, h[MAX_COLS]= {0};
  int sum_w[MAX_COLS]= {0}, sum_h[MAX_COLS]= {0};
  int rows= 0;

  // process stats
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
 
    TRACE("===%2d: i=%1d span=%1d h=%2d w=%2d l=%3d '%s' ... tag=%s\n",
           i, t->i, t->span, t->h, t->w, t->len,t->s?t->s:"(NULL)", t->tag);
  }

  // print stats
  TRACE("\nROWS=%d\n", rows);
  TRACE("\niiii::::: ");
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ", i);
  TRACE("\nWIDTH::: ");
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ", w[i]);

  int a[MAX_COLS]= {0};
  for(int i= 0; i<MAX_COLS; i++)
    a[i]= (sum_w[i]+rows/2)/rows;

  TRACE("\n   AVG:: ");
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ",  a[i]);

  TRACE("\n  A.SUM: "); int width= 0;
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ",  width+=(sum_w[i]+rows/2)/rows);

  TRACE("\n\nHEIGHT:: ");
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ", h[i]);
  TRACE("\n   AVG:: ");
  for(int i= 0; i<MAX_COLS; i++) TRACE("%3d ", (sum_h[i]+rows/2)/rows);
  TRACE("\n---END\n", table->s);
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
    TRACE("=== %d: %d %d\n", i, f[i], d);
  }

  // TODO: if have more space allocate
  // according to "bigger" (title?)
  //
  // shrink, starting with biggest columns
  // TODO: distribute more evenly, maybe
  //   remove 2 from biggest, 1 from next
  int overflows= 0;

  const char *NLCHAR= "↲";

  //const char* sbaseletter="𝗮";
  const char* sbaseletter="₀";
  //const char* sbaseletter="ₐ";
  wchar_t baseletter;
  size_t dummyl;
  u8_to_u32(sbaseletter, strlen(sbaseletter), &baseletter, &dummyl);
  assert(dummyl==1);
  //TRACE("-------baseletter=%x\n", baseletter);

  wchar_t letter[MAX_COLS]= {0};
  
  // init total and tcolums
  int total= 0, tcolumns= 0;
  for(int i=0; i<MAX_COLS; i++) {
    total+= f[i];
    if (f[i]) tcolumns++;
    //TRACE("--%d %d\n", i, f[i]);
  }

  while (total+tcolumns*2+1>screen_cols) {
    TRACE("\n--------MINIMIZE------ %d %d %d\n", total, screen_cols, tcolumns);
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

    // new row
    if (!t->i) {
      putchar('\n');
      // recolor(); // doesn't work?
      B(black); C(white);
      clearend();
    }

    char* s= t->s;
    if (s) {
      if (t->head) {
        B(black); C(white+8);
        underline();
      }

      for(int j=f[t->i]; j>0; j--) {
        //if (*s && *s!=10 && (*s<32 || *s>128)) TRACE("[%d]", *s);
        if (!*s) putchar(' ');
        if (!*s && t->head) {
          B(black); C(white);
          end_underline();
        }
          
        else if (*s == 10) ;
        else if (*s == 13) ;
        else if (*s == (char)HNL) ;
        else if (*s == (char)SNL) printf("↲");
        else if (*s == '\n') {
          do s++; while (*(s+1) && (*s==(char)HNL || *s==(char)SNL));
          printf(NLCHAR);
          putchar(' ');
        }
        else if (*s<32 || *s>127)
          ; // TODO: utf8!!
        else putchar(*s);

        if (*s) s++;
      }
      if (t->head) end_underline();

      // overflow?
      if (!*s) { // not overflow
        putchar(' ');
      } else {
        if (!t->head) {// <TD>
          if (*(s+1)) {
            printf(NLCHAR);
          } else { // last char - print!
            putchar(*s);
          }
          // TODL: save s for next overflow line! print
        } else { // <TH>
          letter[t->i]= baseletter+overflows;
          overflows++;
          putwchar(letter[t->i]);
        } C((int)fg); B((int)bg);
      }
      // extra space between cols
      putchar(' ');
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
    printf("%s\n", t->s);
    // TODO: who is doing newline?
  }
  if (overflows) {
    clearend();
    printf("\n");
  }

  // cleanup
  free(table); table= NULL;

  tdn= 0, tdi= 1, ty=0, tx=0, tp= 0;
  incell= 0;
  memset(tds, 0, sizeof(tds));
}
