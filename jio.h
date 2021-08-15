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
void inverse(int on);
void fgcolor(int c);
void bgcolor(int c);
void savescreen();
void restorescreen();
void insertmode(int on);
void _color(int c);
void fg(int c);
void bg(int c);
int bold(int c /* 0-7 */);
int rgb(int r, int g, int b /* 0-5 */);
int gray(int c /* 0-7 */);
int RGB(int r, int g, int b /* 0-255 */);
void underline();
void end_underline();

// adjusted colors
void C(int n);
void B(int n);

// - higher level colors
enum color{black, red, green, yellow, blue, magnenta, cyan, white, none};

extern enum color _fg,  _bg;

////////////////////////////////////////
// - keyboard

// 'A' CTRL+'A' META+'A' FUNCTION+7
//    0- 31  :  ^@ ^A...
//   32-126  :  ' ' .. 'Z{|}~'
//  127      :  BACKSPACE
// -- Hi bit set == META
//    1- 12  :  F1 .. F12
//   32- 64  : (special keys)
//     M-3     : DEL
//   65- 96  :  M-A .. M-Z
//   97- 126 : (special keys)
//     M-a     : UP
//     M-b     : DOWN
//     M-c     : RIGHT
//     M-d     : LEFT

enum { RETURN='M'-64, TAB='I'-64, ESC=27, BACKSPACE=127, CTRL=-64, META=128, FUNCTION=META, UP=META+'a', DOWN, RIGHT, LEFT, S_TAB=META+'z', DEL=META+'3'};

int key();
void print_key(int c);

int flines(FILE* f);
char* fgetline(FILE* f);
char* fgetlinenum(FILE* f, long line);



