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

// misc system IO

#define TRACE(exprs...) if (trace) printf(exprs);

#define error(exp, exitcode, msg...) \
  do if (exp) { fprintf(stderr, "%%ERROR: %s:%d in %s(...)\n", __FILE__, __LINE__, __func__); fprintf(stderr, msg); fputc('\n', stderr); char*z=NULL; *z=42; kill(getpid(), SIGABRT); exit(exitcode); } while(0)

////////////////////////////////////////
// - ansi screen

extern int screen_rows, screen_cols;

void screen_init();

void reset();
void clear();
void clearend();
void cleareos();

void gotorc(int r, int c);

void cursoroff();
void cursoron();

//void fgcolor(int c);
//void bgcolor(int c);

void inverse(int on);

void savescreen();
void restorescreen();
void insertmode(int on);

// - higher level colors
typedef enum color{black, red, green, yellow, blue, magnenta, cyan, white, none} color;

extern enum color _fg,  _bg;

color fg(int c);
color bg(int c);
int bold(int c /* 0-7 */);
int rgb(int r, int g, int b /* 0-5 */);
int gray(int c /* 0-7 */);
int RGB(int r, int g, int b /* 0-255 */);

void underline();
void end_underline();

// adjusted colors
void recolor();

color C(int n);
color B(int n);

////////////////////////////////////////
// - keyboard

// 'A' CTRL+'A' META+'A' FUNCTION+7
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
enum { RETURN='M'-64, TAB='I'-64, ESC=27, BACKSPACE=127, CTRL=-64, META=128, TERM=512, FUNCTION=META, UP=TERM+'A', DOWN, RIGHT, LEFT, S_TAB=TERM+'Z', DEL=TERM+'3', SHIFT=1024};

int haskey();
int key();

char* keystring(int c);
void testkeys();
char* input(char* prompt);

////////////////////////////////////////
// - files

int flines(FILE* f);
char* fgetline(FILE* f);
char* fgetlinenum(FILE* f, long line);

////////////////////////////////////////
// - strings

int endswith(char* s, char* end);

int isinsideutf8(int c);
int isstartutf8(int c);
int isutf8(int c);

int isfullwidth(int c);
int iszerowidth(int c);

int lprefix(char *a, char *b);
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


////////////////////////////////////////
// Dynamic STRings (see Play/dstrncat.c)

#define DSTR_STEP 64

typedef struct dstr {
  int max;
  char s[0];
} dstr;

dstr* dstrncat(dstr* d, char* add, int n);
dstr* dstrprintf(dstr* d, char* fmt, ...);

///////////////////////////////////
// Date Time functions

char* isotime();

char *timeago(time_t epoch);
char *isoago(char *iso);


