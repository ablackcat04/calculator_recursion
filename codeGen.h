#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

#define NUM_OF_REG 128

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

extern bool isRegAvailable[NUM_OF_REG];
extern int findRegAvailableAndUse();
extern void initReg();

#endif // __CODEGEN__
