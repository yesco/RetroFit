// 
//             jio.h
//             
//          an IO library            
//
//
// (>) CC-BY 2021 Jonas S Karlsson
//
//         jsk@yesco.org
//

#include <stdio.h>
#include <signal.h>

// misc system IO

// call in main, first thing
void jio(); 

void _jio_exit(); // atexit() should do



// DEBUG; will cause interrupt and go gdb
#define DEBUG raise(SIGTRAP)

#define TRACE(exprs...) if (trace) printf(exprs);

#define error(exp, exitcode, msg...) \
  do if (exp) { fprintf(stderr, "%%ERROR: %s:%d in %s(...)\n", __FILE__, __LINE__, __func__); fprintf(stderr, msg); fputc('\n', stderr); char*z=NULL; *z=42; kill(getpid(), SIGABRT); exit(exitcode); } while(0)

// timestamped logging

#define FLOGF(F, fmt, args...) \
  do { \
   dstr* d= dstrprintf(NULL, "%s " fmt, isotime(), args); \
   if (F) fputs(d->s, F); \
   if (F) fflush(F); \
   if (d) free(d); \
  } while(0)

////////////////////////////////////////
// - ansi screen

extern int screen_rows, screen_cols;

void screen_init(int sig);

void reset();
void clear();
void clearend();
void cleareos();

// notice thi sis zero based! (not ansi)
void gotorc(int r, int c);

void cursoroff();
void cursoron();

void save();
void restore();

//void fgcolor(int c);
//void bgcolor(int c);

void inverse(int on);

void savescreen();
void restorescreen();
void insertmode(int on);

// - higher level colors
// ( https://www.ditig.com/publications/256-colors-cheat-sheet )
// ( https://www.calmar.ws/vim/256-xterm-24bit-rgb-color-chart.html )
// ( https://raw.githubusercontent.com/jonasjacek/colors/master/index.html _
typedef enum color{
  black, red, green, yellow, blue, magenta, cyan, white,
  none=8, // TODO: not right? -1? -2? lol (byte... hmmm)
  // 8--15: same but bright!
  // 16--231: rgb= 16 + 36*r + 6*g + b  USE: rgb(r,g,b)
  // 231--255: gray / 24 ???
  // jsk's guesses
  purple= 93,
  orange= 214,
  brightorange= yellow+8,

  Black            = 0,
  Maroon           = 1,
  Green            = 2,
  Olive            = 3,
  Navy             = 4,
  Purple           = 5,
  Teal             = 6,
  Silver           = 7,
  Grey             = 8,
  Red              = 9,
  Lime             = 10,
  Yellow           = 11,
  Blue             = 12,
  Fuchsia          = 13,
  Aqua             = 14,
  White            = 15,
  
  // ----------------
  // These are mostly xterm/ANSI colors
  // They don't have unique names, neither number after name!
  // I've uniqieufied them and roughtly sorted in basic colors
  // But nothing beats looking at it and picking it by name/number.


  // -- GREEN

  Green1           = 46,
  Green2           = 34,
  Green3           = 40,
  Green4           = 28,

  GreenYellow      = 154,

  DarkGreen        = 22,

  DarkOliveGreen1  = 191,
  DarkOliveGreen2  = 155,
  DarkOliveGreen3  = 113,
  DarkOliveGreen4  = 192,
  DarkOliveGreen5  = 149,
  DarkOliveGreen6  = 107,

  DarkSeaGreen     = 108,
  DarkSeaGreen1    = 158,
  DarkSeaGreen2    = 157,
  DarkSeaGreen3    = 150,
  DarkSeaGreen4    = 65,
  DarkSeaGreen5    = 115,
  DarkSeaGreen6    = 151,
  DarkSeaGreen7    = 193,
  DarkSeaGreen8    = 71,

  Khaki1           = 228,
  Khaki3           = 185,

  DarkKhaki        = 143,

  LightGreen       = 119,
  LightGreen2      = 120,

  LightSeaGreen    = 37,

  MediumSpringGreen = 49,

  PaleGreen1       = 156,
  PaleGreen2       = 114,
  PaleGreen3       = 77,
  PaleGreen4       = 121,

  SeaGreen1        = 84,
  SeaGreen2        = 83,
  SeaGreen4        = 85,
  SeaGreen3        = 78,

  SpringGreen1     = 48,
  SpringGreen2     = 42,
  SpringGreen5     = 47,
  SpringGreen3     = 35,
  SpringGreen6     = 41,
  SpringGreen4     = 29,

  Honeydew2        = 194, // ?


  // -- RED

  Red1             = 196,
  Red2             = 160,
  Red3             = 124,

  DarkRed          = 52,
  DarkRed2         = 88,

  IndianRed        = 131,
  IndianRed1       = 203,
  IndianRed2       = 204,
  IndianRed3       = 167,

  MediumVioletRed  = 126,

  HotPink          = 205,
  HotPink1         = 206,
  HotPink2         = 169,
  HotPink3         = 168,
  HotPink4         = 132,

  Magenta1         = 201,
  Magenta2         = 200,
  Magenta3         = 163,
  Magenta4         = 127,
  Magenta5         = 164,
  Magenta6         = 165,

  DarkMagenta      = 90,
  DarkMagenta2     = 91,

  MistyRose1       = 224,
  MistyRose3       = 181,

  PaleVioletRed1   = 211,

  Pink1            = 218,
  Pink3            = 175,

  DeepPink1        = 198,
  DeepPink2        = 197,
  DeepPink3        = 161,
  DeepPink4        = 125,
  DeepPink5        = 162,
  DeepPink6        = 199,
  DeepPink7        = 53,
  DeepPink8        = 89,

  Orange1          = 214,
  Orange2          = 58,
  Orange3          = 172,
  Orange4          = 94,

  OrangeRed1       = 202,


  // -- BLUE 

  Aquamarine1      = 86,
  Aquamarine2      = 122,
  Aquamarine3      = 79,

  Blue1            = 21,
  Blue2            = 19,
  Blue3            = 20,

  BlueViolet       = 57,

  CadetBlue        = 72,
  CadetBlue2       = 73,

  Chartreuse1      = 118,
  Chartreuse2      = 112,
  Chartreuse3      = 70,
  Chartreuse4      = 64,
  Chartreuse5      = 76,
  Chartreuse6      = 82,

  CornflowerBlue   = 69,

  Cornsilk1        = 230,

  Cyan1            = 51,
  Cyan2            = 50,
  Cyan3            = 43,

  DarkBlue2        = 18,

  DarkCyan         = 36,

  LightSkyBlue1    = 153,
  LightSkyBlue2    = 109,
  LightSkyBlue3    = 110,

  LightSlateBlue   = 105,

  LightSteelBlue   = 147,
  LightSteelBlue1  = 189,
  LightSteelBlue3  = 146,

  DeepSkyBlue1     = 39,
  DeepSkyBlue2     = 38,
  DeepSkyBlue3     = 31,
  DeepSkyBlue5     = 32,
  DeepSkyBlue4     = 23,
  DeepSkyBlue6     = 24,
  DeepSkyBlue7     = 25,

  DodgerBlue1      = 33,
  DodgerBlue2      = 27,
  DodgerBlue3      = 26,

  NavyBlue         = 17,

  RoyalBlue1       = 63,

  SkyBlue1         = 117,
  SkyBlue2         = 111,
  SkyBlue3         = 74,

  SlateBlue1       = 99,
  SlateBlue2       = 61,
  SlateBlue3       = 62,

  SteelBlue        = 67,
  SteelBlue1       = 68,
  SteelBlue2       = 75,
  SteelBlue4       = 81,

  LightCoral       = 210,

  LightCyan1       = 195,
  LightCyan3       = 152,

  LightPink1       = 217,
  LightPink3       = 174,
  LightPink4       = 95,

  LightSalmon1     = 216,
  LightSalmon2     = 137,
  LightSalmon3     = 173,

  MediumOrchid     = 134,
  MediumOrchid1    = 171,
  MediumOrchid2    = 207,
  MediumOrchid3    = 133,

  MediumPurple     = 104,
  MediumPurple1    = 141,
  MediumPurple2    = 140,
  MediumPurple3    = 97,
  MediumPurple4    = 60,
  MediumPurple5    = 98,
  MediumPurple6    = 135,

  MediumTurquoise  = 80,

  Orchid           = 170,
  Orchid1          = 213,
  Orchid2          = 212,

  PaleTurquoise1   = 159,
  PaleTurquoise4   = 66,

  Turquoise2       = 45,
  Turquoise4       = 30,

  Violet           = 177,

  DarkTurquoise    = 44,

  DarkViolet2      = 128,
  DarkViolet3      = 92,

  Plum1            = 219,
  Plum2            = 183,
  Plum3            = 176,
  Plum4            = 96,

  Purple2          = 129,
  Purple3          = 56,
  Purple4          = 54,
  Purple1          = 55,
  Purple5          = 93,


  // -- YELLOW

  Yellow1          = 226,
  Yellow2          = 190,
  Yellow3          = 184,
  Yellow4          = 100,
  Yellow5          = 106,
  Yellow6          = 148,

  LightYellow3     = 187,

  Gold1            = 220,
  Gold2            = 142,
  Gold3            = 178,

  DarkGoldenrod    = 136,

  DarkOrange       = 208,
  DarkOrange2      = 166,
  DarkOrange3      = 130,

  LightGoldenrod1  = 227,
  LightGoldenrod2  = 221,
  LightGoldenrod3  = 179,
  LightGoldenrod4  = 222,
  LightGoldenrod5  = 186,

  
  // -- BROWN
  
  RosyBrown        = 138,

  Salmon1          = 209,

  SandyBrown       = 215,

  Tan              = 180,

  Thistle1         = 225,
  Thistle3         = 182,

  Wheat1           = 229,
  Wheat4           = 101,

  // -- GRAY GREY

  NavajoWhite1     = 223,
  NavajoWhite3     = 144,

  DarkSlateGray1   = 123,
  DarkSlateGray2   = 87,
  DarkSlateGray3   = 116,

  LightSlateGrey   = 103,

  Grey0            = 16,
  Grey37           = 59,
  Grey53           = 102,
  Grey63           = 139,
  Grey69           = 145,
  Grey84           = 188,
  Grey100          = 231,

  Grey3            = 232,
  Grey7            = 233,
  Grey11           = 234,
  Grey15           = 235,
  Grey19           = 236,
  Grey23           = 237,
  Grey27           = 238,
  Grey30           = 239,
  Grey35           = 240,
  Grey39           = 241,
  Grey42           = 242,
  Grey46           = 243,
  Grey50           = 244,
  Grey54           = 245,
  Grey58           = 246,
  Grey62           = 247,
  Grey66           = 248,
  Grey70           = 249,
  Grey74           = 250,
  Grey78           = 251,
  Grey82           = 252,
  Grey85           = 253,
  Grey89           = 254,
  Grey93           = 255,

} color;

extern enum color _fg,  _bg;
extern int _reverse;

color fg(int c);
color bg(int c);
int bold(int c /* 0-7 */);
int rgb(int r, int g, int b /* 0-5 */);
int gray(int c /* 0-7 */);
int RGB(int r, int g, int b /* 0-255 */);

void boldon();
void boldoff();

void underline();
void end_underline();

// adjusted colors
void recolor();

color C(int n);
color B(int n);

color invertcolor(color c);

color readablefg();

////////////////////////////////////////
// Macros for array length etc

#define ALEN(array) (sizeof(array)/sizeof(*array))
// TODO: bad name
// Use: to pick an element from an array by percentage row/rows
#define AREF(array, percent) ((array)[ALEN(array)*percent])

#define SQR(a) ((a)*(a))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ABS(a) ((a)<0?-(a):(a))

// free:s variable VAR if not null, and sets it to NULL.
// Note: var is evaluated maybe 3 times...
#define FREE(var) (var?free(var):0, var=NULL)

////////////////////////////////////////
// Dynamic STRings (see Play/dstrncat.c)

#define DSTR_STEP 64

typedef struct dstr {
  int max;
  char s[0];
} dstr;

dstr* dstrncat(dstr* d, char* add, int n);
dstr* dstrprintf(dstr* d, char* fmt, ...);
dstr *dstrncaturi(dstr *d, char *s, int len);

////////////////////////////////////////
// - keyboard

// 'A' CTRL+'A' META+'A' FUNC+7
//    0- 31  :  ^@ ^A...
//   32-126  :  ' ' .. 'Z{|}~'
//  127      :  BACKSPACE
// -- Hi bit set == META
//    1- 12  :  F1 .. F12
//   32- 64  : (special keys)
//   65- 96  :  M-A .. M-Z
// -- 256 (9th bit set) = TERM
//   TERM+'A': UP
//        'B': DOWN
//        'C': RIGHT
//        'D': LEFT
//   TERM+'Z': DEL
typedef enum keycode { RETURN='M'-64, TAB='I'-64, ESC=27, BACKSPACE=127,
  CTRL=-64, META=256, FUNC=META, ALT=META, TERM=1024,
  UP=TERM+'A', DOWN, RIGHT, LEFT, S_TAB=TERM+'Z', DEL=TERM+'3', SHIFT=1024,
  CTRLX=2048, /* not used */
  MOUSE_DOWN=0x01000000, MOUSE_UP=MOUSE_DOWN*2, MOUSE=MOUSE_DOWN+MOUSE_UP, SCROLL_DOWN=0x04000000, SCROLL_UP=0x08000000, SCROLL=SCROLL_DOWN+SCROLL_UP } keycode;

extern keycode _prevkey, _lastkey;
extern long _prevkeyms, lastkeyms;

int haskey();
keycode key();
keycode peekey();
int keywait(int ms);
keycode waitScrollEnd(keycode k);
int keyRepeated();

int mouserow(int k);
int mousecol(int k);

char* keystring(int c);
void testkeys();

// TODO: remove: use edit!
char* input(char* prompt);

keycode edit(dstr **dsp, int width, char *allowed, char *not, char *breaks);

////////////////////////////////////////
// - files

int flines(FILE* f);
char* fgetline(FILE* f);
char* fgetlinenum(FILE* f, long line);

////////////////////////////////////////
// - strings

int fscan(FILE *f, char *till);

void qprintstr(char *s);

int endswith(char* s, char* end);

int isinsideutf8(int c);
int isstartutf8(int c);
int isutf8(int c);

int isfullwidth(int c);
int iszerowidth(int c);

int lprefix(char *a, char *b);
char *skipspc(char *p);
char *sskip(char *s, char *w);
char *struncp(char *s, char *p);
char *strunc(char *s, char *w);
char *strunch(char *s, char c);
char *struncafter(char *s, char *w, char c);
char *sdel(char *s, char *w);
char *sdelall(char *s, char *w);
char *srepl(char *s, char *w, char c);
char *sreplbetween(char *s, char *first, char *last, char c, int keep);
char *scollapse(char *s, char c, int n);

///////////////////////////////////
// Date Time functions

char* isotime();

char *timeago(time_t epoch);
char *isoago(char *iso);

long utime();
long mstime();

///////////////////////////////////
// string goodies

void spc();
void bs();
void repeat(char c, int n);
void spaces(int n);

//////////////////////////////
// string extractors

char *getcolonval(char *field, char *s);


