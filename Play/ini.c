#include <stdio.h>
#include <assert.h>

#include "../jio.c"

#define QUOTE(sym) #sym
#define LENGTH(arr) (sizeof(arr)/sizeof(*arr))

// create the INI variables
#define INI(type, name, value) type name= value;
#define INT(name, value) INI(int, name, value)
#define STR(name, value) INI(char*, name, NULL)
#include "ini.def"

void resetini() {
// (re)init the INI variables
#define INI(type, name, value) name= value;
#define INT(name, value) INI(int, name, value)
#define STR(name, value) if (name) free(name); INI(char*, name, strdup(value))
#include "ini.def"
}

// create an array of names & pointers
#define INI(itype, iname, ivalue) {.name=QUOTE(iname), .pvar=&iname, .type=QUOTE(itype) },
#define INT(iname, ivalue) INI(int, iname, ivalue)
#define STR(iname, ivalue) INI(char*, iname, NULL)
struct inivar {
  char *name;
  void *pvar;
  char* type;
} inivars[] = {
#include "ini.def"
};

int setini(int i, char *name, int len, char *line) {
  fprintf(stderr, "--SETINI %d '%s' %d \"%s\"\n", i, name, len, line);
  // -- find value after '=' or/and whitespace
  char *p = line+strlen(inivars[i].name);
  if (*p=='=') p++;
  while(*p && isspace(*p)) p++;
  // verify = or space
  if (p==line) return 0;

  // -- set variable
  if (inivars[i].type=="int") {
    *(int*)(inivars[i].pvar) = atoi(p);
  } else if (inivars[i].type=="char*") {
    char *s = *(char**)(inivars[i].pvar);
    // TODO: how to handle first value?
    if (s) free(s);
    *(char**)(inivars[i].pvar)= strdup(p);
  } else {
    // either your C-compiler doesn't intern strings or type is not int/char*
    assert(inivars[i].type=="type not recognized!");
  }
  return 1;
}

//#include "ini.def"

int loadini(char *fname) {
  FILE *f= fopen(fname, "r");
  if (!f) return 0;
  char *line;
  int ln= 0;
  while((line= fgetline(f))) {
    ln++;
    // find variable
    int i, len= 0;
    char *name= NULL;
    for(i=0; i<LENGTH(inivars); i++) {
      name= inivars[i].name;
      len= strlen(name);
      // found?
      if (!strncmp(line, name, len)) {
        if (!setini(i, name, len, line))
          break;
        goto next; // yeah goto!
      }
    }
    // not found
    fprintf("%% loadini %s:%d: variable in line \"%s\" not recognized (or delimiter missing)\n", fname, ln, line);

  next: 
    free(line);
  }
  fclose(f);
  return 1;
}

void printini() {
  for(int i=0; i<LENGTH(inivars); i++) {
    printf("%d: %s=", i, inivars[i].name);
    // rely on compiler interned strings!
    if (inivars[i].type=="int") {
      printf("%d\n", *(int*)(inivars[i].pvar));
    } else if (inivars[i].type=="char*") {
      char *s = *(char**)(inivars[i].pvar);
      printf("%s\n", s? s: "(null)");
    } else {
      // either your C-compiler doesn't intern strings or type is not int/char*
      assert(inivars[i].type=="type not recognized!");
    }
  }
}

int main(void) {
  printf("BG_body=%d\n", BG_body);
  printf("default_profile=%s\n", default_profile);

  printini();

  if (!loadini("Play/ini.ini")) {
    fprintf(stderr, "%% Loading ini.ini failed!\n");
  }
  printini();
  resetini();
  printini();
}


