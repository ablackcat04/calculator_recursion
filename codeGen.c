#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

Register r[8];

Reassembly newReassmbly(int rtvl, DataPlacementSet placement, int regi, int cons)
{
    Reassembly R;
    R.return_value = rtvl;
    R.placement = placement;
    R.regist = regi;
    R.constant = cons;

    return R;
}

int inRegister(char* str)
{
    for (int i = 0; i < 8; ++i) {
        if (r[1].is_id && strcmp(str, r[i].id) == 0)
        {
            printf("Find %s in register%d\n", str, i);
            return i;
        }
    }
    return -1;
}

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

Reassembly generateAssembly(BTNode *root)
{
    if (root != NULL)
    {
        int i;
        Reassembly R;
        switch (root->data) {
            case ASSIGN:
                for (i = 0; i < 8; ++i) {
                    if (r[i].is_id && strcmp(r[i].id, root->left->lexeme) == 0)
                    {
                        printf("yeah! r%d=%s\n", i, root->left->lexeme);

                        R = generateAssembly(root->right);

                        if (R.placement == CONST)
                        {
                            printf("MOV r%d %d\n", i, R.constant);
                        }
                        else if (R.placement == REGISTER)
                        {
                            printf("MOV r%d r%d\n", i, R.regist);
                        }

                        R.placement = REGISTER;
                        R.regist = i;

                        return R;
                    }
                }

                break;
            case INT:
                R = newReassmbly(atoi(root->lexeme), CONST, -1, atoi(root->lexeme));
                printf("RRRR %d\n", R.constant);
                return R;
            case ID:
                if (inRegister(root->lexeme) != -1)
                {
                    R = newReassmbly(getval(root->lexeme), REGISTER, inRegister(root->lexeme), -1);
                }
                else
                {
                    printf("MOV ra %d\n", getval(root->lexeme));
                }

                return R;
            case ADDSUB:
            case INCDEC:
            case MULDIV:
            case ADDSUB_ASSIGN:
            case AND:
            case OR:
            case XOR:
            default:
                printf("Nooo\n");
                R = newReassmbly(0, UNDEF, -1, -1);
                return R;
        }
    }
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
