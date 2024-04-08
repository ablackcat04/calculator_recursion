#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"
#include <stdbool.h>

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

extern void initRegister(void);

#endif // __CODEGEN__
