#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

Register r[8];

void initRegister()
{
    r[0].is_id = true;
    r[1].is_id = true;
    r[2].is_id = true;
    r[3].is_id = false;
    r[4].is_id = false;
    r[5].is_id = false;
    r[6].is_id = false;
    r[7].is_id = false;

    r[0].value = getval("x");
    r[1].value = getval("y");
    r[2].value = getval("z");
    r[3].value = 0;
    r[4].value = 0;
    r[5].value = 0;
    r[6].value = 0;
    r[7].value = 0;

    strcpy(r[0].id, "x");
    strcpy(r[1].id, "y");
    strcpy(r[2].id, "z");

    printf("MOV r0 [0]\n");
    printf("MOV r1 [4]\n");
    printf("MOV r2 [8]\n");
}

int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                break;
            case INT:
                retval = atoi(root->lexeme);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                break;
            case ADDSUB:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                }
                break;
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    //TODO: deal with div by zero error separately, if right side has variable don't print EXIT(1)

                    if (rv == 0)
                        error(DIVZERO);
                    retval = lv / rv;
                }
                break;
            case OR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv | rv;
                break;
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv & rv;
                break;
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval = lv ^ rv;
                break;
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if(strcmp(root->lexeme, "+=") == 0)
                    retval = setval(root->left->lexeme, lv+rv);
                else
                    retval = setval(root->left->lexeme, lv-rv);
                break;
            case INCDEC:
                if (strcmp(root->lexeme, "++a") == 0) {
                    retval = setval(root->left->lexeme, evaluateTree(root->left) + 1) - 1;
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "++b") == 0) {
                    retval = setval(root->left->lexeme, evaluateTree(root->left) + 1);
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "--a") == 0) {
                    retval = setval(root->left->lexeme, evaluateTree(root->left) - 1) + 1;
                    strcpy(root->lexeme, "--");
                } else if (strcmp(root->lexeme, "--b") == 0) {
                    retval = setval(root->left->lexeme, evaluateTree(root->left) - 1);
                    strcpy(root->lexeme, "--");
                } else {
                    retval = evaluateTree(root->left);
                }

                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
