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
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                } else if (strcmp(root->lexeme, "/") == 0) {
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
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if(strcmp(root->lexeme, "+=") == 0)
                    retval = setval(root->left->lexeme, lv+rv);
                else
                    retval = setval(root->left->lexeme, lv-rv);
                break;
            case INCDEC:
                if (strcmp(root->lexeme, "+a") == 0) {
                    printf("+a\n");
                    retval = setval(root->left->lexeme, evaluateTree(root->left)+1)-1;
                } else if (strcmp(root->lexeme, "+p") == 0) {
                    printf("+p\n");
                    retval = setval(root->left->lexeme, evaluateTree(root->left)+1);
                } else if (strcmp(root->lexeme, "-a") == 0) {
                    printf("-a\n");
                    retval = setval(root->left->lexeme, evaluateTree(root->left)-1)+1;
                } else {
                    printf("-p\n");
                    retval = setval(root->left->lexeme, evaluateTree(root->left)-1);
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
