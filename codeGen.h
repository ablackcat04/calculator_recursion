#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"
#include <stdbool.h>
#include "lex.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);



typedef struct regi
{
    bool is_id;
    char id[MAXLEN];
    int value;
} Register;

typedef struct reasm
{
    int return_value;
    DataPlacementSet placement;
    int regist;
    int constant;
} Reassembly;

// Generate the assembly code for this expression
extern Reassembly generateAssembly(BTNode *root);

extern Reassembly newReassmbly(int, DataPlacementSet, int, int);

extern void initRegister(void);

extern int inRegister(char*);

extern int putInResister(char*);

extern int findFreeRegister(void);

#endif // __CODEGEN__
