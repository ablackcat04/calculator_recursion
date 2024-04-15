#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

bool isRegAvailable[NUM_OF_REG];

void initReg() {
    for (int i = 0; i < NUM_OF_REG; ++i) {
        isRegAvailable[i] = true;
    }
}

int findRegAvailableAndUse() {
    for (int i = 0; i < NUM_OF_REG; ++i) {
        if (isRegAvailable[i]) {
            isRegAvailable[i] = false;
            return i;
        }
    }
    return -1;
}


ReturnType evaluateTree(BTNode *root) {
    //int lv = 0, rv = 0;
    ReturnType retval, lv, rv;
    retval.rtvl = 0;
    lv.rtvl = 0;
    rv.rtvl = 0;

    int rx = 0;
    int ry = 0;
    char command[3];

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval.rtvl = getval(root->lexeme);
                retval.type = VAR;
                retval.value = getmem(root->lexeme);
                //printf("ID [%d]\n", retval.value);
                break;
            case INT:
                retval.rtvl = atoi(root->lexeme);
                retval.type = CONST;
                retval.value = retval.rtvl;
                //printf("INT %d\n", retval.value);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval.rtvl = setval(root->left->lexeme, rv.rtvl);
                retval.type = REG;

                ry = findRegAvailableAndUse();
                if (rv.type == VAR) {
                    printf("MOV r%d [%d]\n", ry, rv.value);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else if (rv.type == CONST) {
                    printf("MOV r%d %d\n", ry, rv.value);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else if (rv.type == REG) {
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else {
                    printf("Warning: rv without type\n");
                }
                isRegAvailable[ry] = true;

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
                    rx = lv.value;
                    if (rv.type == REG) {
                        ry = rv.value;
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == VAR) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d [%d]\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == CONST) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d %d\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else {
                        printf("Warning: rv without type\n");
                    }
                    retval.value = rx;
                } else if (lv.type == VAR) {
                    rx = findRegAvailableAndUse();
                    if (rv.type == REG) {
                        ry = rv.value;
                        printf("MOV r%d [%d]\n", rx, lv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == VAR) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d [%d]\n", rx, lv.value);
                        printf("MOV r%d [%d]\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == CONST) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d [%d]\n", rx, lv.value);
                        printf("MOV r%d %d\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else {
                        printf("Warning: rv without type\n");
                    }
                    retval.value = rx;
                } else if (lv.type == CONST) {
                    rx = findRegAvailableAndUse();
                    if (rv.type == REG) {
                        ry = rv.value;
                        printf("MOV r%d %d\n", rx, lv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == VAR) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d %d\n", rx, lv.value);
                        printf("MOV r%d [%d]\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else if (rv.type == CONST) {
                        ry = findRegAvailableAndUse();
                        printf("MOV r%d %d\n", rx, lv.value);
                        printf("MOV r%d %d\n", ry, rv.value);
                        printf("%s r%d r%d\n", command, rx, ry);
                        isRegAvailable[ry] = true;
                    } else {
                        printf("Warning: rv without type\n");
                    }
                    retval.value = rx;
                } else {
                    printf("Warning: lv without type\n");
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
