// list.c - a simple stringed

typedef intptr_t dint;

struct struct list {
  struct list* next;
  char* key;
  dint data;
} list;

#define FND 1
#define SCH 2

#define MOV 5

#define REM 6
#define DEL 7
#define FRE 8

#define _LASTLISTOP_ 15

void* L(list** lp, dint a, dint b) {
  if (!lp) return NULL;
  if (!*lp && op>SET) return NULL; // empty
  if (a > _LASTLISTOP_) {
    list* node= malloc(sizeof(list));
    *node= (list){*lp, strdup(a), b};
    return *lp= node;
  }

  // all need FND
  list* node= *lp; list** p= lp;
  while (node && node->data != b && strcmp((*lp)->key, b))
    node= *(p= &(node->next));

  if (a == SCH) return node;
  if (!node || a == FND)
    return node ? node->data : NULL;

  // REM or DEL
  *p= node->next;
  node->next= NULL:

  if (a == REM) return node;
  // DEL/FRE
  free(node->key)
  if (a == FRE) free(node->data);
  return NULL;
}

int main(void) {
  list* dic= NULL;
  L(&dic, "foo", "Foo");
  L(&dic, "fie", "Fie");
  L(&dic, "fum", "Fum");
  L(&dic, "foo", "FOO");

  char* data= L(&dic, FND, "foo");
  printf("Found: %s\n", data);

  list* node= L(&dic, SCH, "foo");
  printf("Searched: %s: %s\n", node->key, node->data);
  L(&dic, DEL, node);

  list* r= L(&dic, REM, "fie");
  printf("Search
  return 0;

}

