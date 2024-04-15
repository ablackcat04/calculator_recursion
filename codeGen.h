#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

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


#endif // __CODEGEN__
