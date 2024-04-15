#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

ReturnType evaluateTree(BTNode *root) {
    //int lv = 0, rv = 0;
    ReturnType retval, lv, rv;
    retval.rtvl = 0;
    lv.rtvl = 0;
    rv.rtvl = 0;

    char command[3];

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval.rtvl = getval(root->lexeme);
                retval.type = VAR;
                retval.value = getmem(root->lexeme);
                //printf("[%d]\n", retval.value);
                break;
            case INT:
                retval.rtvl = atoi(root->lexeme);
                retval.type = CONST;
                retval.value = retval.rtvl;
                //printf("%d\n", retval.value);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval.rtvl = setval(root->left->lexeme, rv.rtvl);
                retval.type = REG;

                if (rv.type == VAR) {
                    printf("MOV rx [%d]\n", rv.value);
                    printf("MOV [%d] rx\n", getmem(root->left->lexeme));
                } else if (rv.type == CONST) {
                    printf("MOV rx %d\n", rv.value);
                    printf("MOV [%d] rx\n", getmem(root->left->lexeme));
                } else if (rv.type == REG) {
                    printf("MOV [%d] rx\n", getmem(root->left->lexeme));
                }

                break;
            case ADDSUB:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval.rtvl = lv.rtvl + rv.rtvl;
                    strcpy(command, "ADD");
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval.rtvl = lv.rtvl - rv.rtvl;
                    strcpy(command, "SUB");
                }
                retval.type = REG;

                if (lv.type == REG) {
                    if (rv.type == REG) {
                        printf("%s rx ry\n", command);
                    } else if (rv.type == VAR) {
                        printf("MOV ry [%d]\n", rv.value);
                        printf("%s rx ry\n", command);
                    } else if (rv.type == CONST) {
                        printf("MOV ry %d\n", rv.value);
                        printf("%s rx ry\n", command);
                    }
                } else if (lv.type == VAR) {
                    if (rv.type == REG) {
                        printf("MOV rx [%d]\n", lv.value);
                        printf("%s rx ry\n", command);
                    } else if (rv.type == VAR) {
                        printf("MOV rx [%d]\n", lv.value);
                        printf("MOV ry [%d]\n", rv.value);
                        printf("%s rx ry\n", command);
                    } else if (rv.type == CONST) {
                        printf("MOV rx [%d]\n", lv.value);
                        printf("MOV ry %d\n", rv.value);
                        printf("%s rx ry\n", command);
                    }
                } else if (lv.type == CONST) {
                    if (rv.type == REG) {
                        printf("MOV rx %d\n", lv.value);
                        printf("%s rx ry\n", command);
                    } else if (rv.type == VAR) {
                        printf("MOV rx %d\n", lv.value);
                        printf("MOV ry [%d]\n", rv.value);
                        printf("%s rx ry\n", command);
                    } else if (rv.type == CONST) {
                        printf("MOV rx %d\n", lv.value);
                        printf("MOV ry %d\n", rv.value);
                        printf("%s rx ry\n", command);
                    }
                }

                break;
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "*") == 0) {
                    retval.rtvl = lv.rtvl * rv.rtvl;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    //TODO: deal with div by zero error separately, if right side has variable don't print EXIT(1)

                    if (rv.rtvl == 0)
                        error(DIVZERO);
                    retval.rtvl = lv.rtvl / rv.rtvl;
                }

                break;
            case OR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl | rv.rtvl;

                break;
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl & rv.rtvl;

                break;
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl ^ rv.rtvl;

                break;
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if(strcmp(root->lexeme, "+=") == 0)
                    retval.rtvl = setval(root->left->lexeme, lv.rtvl + rv.rtvl);
                else
                    retval.rtvl = setval(root->left->lexeme, lv.rtvl - rv.rtvl);

                break;
            case INCDEC:
                if (strcmp(root->lexeme, "++a") == 0) {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl + 1) - 1;
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "++b") == 0) {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl + 1);
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "--a") == 0) {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl - 1) + 1;
                    strcpy(root->lexeme, "--");
                } else if (strcmp(root->lexeme, "--b") == 0) {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl - 1);
                    strcpy(root->lexeme, "--");
                } else {
                    retval.rtvl = evaluateTree(root->left).rtvl;
                }

                break;
            default:
                retval.rtvl = 0;
        }
    }
    return retval;
}

int oldEvaluateTree(BTNode *root) {
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
                rv = oldEvaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                break;
            case ADDSUB:
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                }
                break;
            case MULDIV:
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
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
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
                retval = lv | rv;
                break;
            case AND:
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
                retval = lv & rv;
                break;
            case XOR:
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
                retval = lv ^ rv;
                break;
            case ADDSUB_ASSIGN:
                lv = oldEvaluateTree(root->left);
                rv = oldEvaluateTree(root->right);
                if(strcmp(root->lexeme, "+=") == 0)
                    retval = setval(root->left->lexeme, lv+rv);
                else
                    retval = setval(root->left->lexeme, lv-rv);
                break;
            case INCDEC:
                if (strcmp(root->lexeme, "++a") == 0) {
                    retval = setval(root->left->lexeme, oldEvaluateTree(root->left) + 1) - 1;
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "++b") == 0) {
                    retval = setval(root->left->lexeme, oldEvaluateTree(root->left) + 1);
                    strcpy(root->lexeme, "++");
                } else if (strcmp(root->lexeme, "--a") == 0) {
                    retval = setval(root->left->lexeme, oldEvaluateTree(root->left) - 1) + 1;
                    strcpy(root->lexeme, "--");
                } else if (strcmp(root->lexeme, "--b") == 0) {
                    retval = setval(root->left->lexeme, oldEvaluateTree(root->left) - 1);
                    strcpy(root->lexeme, "--");
                } else {
                    retval = oldEvaluateTree(root->left);
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
