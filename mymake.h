#ifndef _MYMAKE_H
#define _MYMAKE_H
#include <stdio.h>
//Typedefs
typedef struct pat {
  char *str;
  struct pat *next;
} path;

typedef struct tar {
  char *str;
  struct tar *next;
} targets;

typedef struct com {
  char *str;
  struct com *next;
} command;

typedef struct n {
  char *name;
  struct tar *tars;
  struct e *deps;
  int visited;
  int target;
  command *commands;
  struct n *next;
} node;

typedef struct e {
  node *dep;
  struct e *next;
} edge;

typedef struct urm {
  char *name;
  char *string;
  struct urm *next;
} usermacro;

// Global 
extern node *head;
extern usermacro *head_mac;
extern path *head_path;

// Flag related
extern int p_flag;
extern int k_flag;
extern int d_flag;
extern int i_flag;
extern int t_num;

//prototypes from graphstruct.c
targets *addTarget(node *, char *);
path *processPath(char *);
usermacro *addMacro(char *, char *);
char *findMacro(char *);
node *addNode(char *);
command *addCommand(node *, char *);
edge *addEdge(node *, char *);
node *findNode(char *);
void travGraph(char *);
void freeGraph();
void printStruct();

// prototypes from processFile.c
int readFile(FILE *);

// prototypes from processCommand.c
int process_command(char *);

#endif