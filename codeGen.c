#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

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
