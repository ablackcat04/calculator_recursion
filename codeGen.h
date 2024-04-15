#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

typedef enum rs
{
    VAR, CONST, REG
}ReturnSet;

typedef struct rtt
{
    int rtvl;
    ReturnSet type;
    int value;
}ReturnType;

// Evaluate the syntax tree
extern ReturnType evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);




#endif // __CODEGEN__
