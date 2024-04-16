#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

bool is_new_var(char *str) {
    for (int i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0)
            return false;
    }
    return true;
}

int getmem(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0)
            return i*4;
    }
    return -1;
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return 0;
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

/*
statement        := ENDFILE | END | assign_expr END
assign_expr      := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr
or_expr          := xor_expr or_expr_tail
or_expr_tail     := OR xor_expr or_expr_tail | NiL
xor_expr         := and_expr xor_expr_tail
xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
and_expr         := addsub_expr and_expr_tail
and_expr_tail    := AND addsub_expr and_expr_tail | NiL
addsub_expr      := muldiv_expr addsub_expr_tail
addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
muldiv_expr      := unary_expr muldiv_expr_tail
muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
unary_expr       := ADDSUB unary_expr | factor
factor           := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
 */

//statement        := ENDFILE | END | assign_expr END
void statement(void)
{
    BTNode *retp = NULL;

    if (match(ENDFILE)) {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            printf("%d\n", evaluateTree(retp).rtvl);
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

//assign_expr      := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr
BTNode *assign_expr(void)
{
    BTNode *retp = NULL, *left = NULL;

    if (match(ID))
    {
        left = makeNode(ID, getLexeme());
        advance();

        if (match(ASSIGN))
        {

            retp = makeNode(ASSIGN, getLexeme());
            retp->left = left;
            advance();
            retp->right = assign_expr();

        }
        else if (match(ADDSUB_ASSIGN))
        {

            if (is_new_var(left->lexeme))
                error(SYNTAXERR);

            retp = makeNode(ADDSUB_ASSIGN, getLexeme());
            retp->left = left;
            advance();
            retp->right = assign_expr();
        }
        else
        {
            int len = (int)strlen(getLexeme());
            char str[len + 5];
            strcpy(str, getLexeme());

            if(len == 0)
            {
                ungetc('\n', stdin);
            }
            else
            {
                for (int i = len - 1; i >= 0; i--)
                {
                    ungetc(str[i], stdin);
                }
            }

            len = (int)strlen(left->lexeme);

            for (int i = len-1; i >= 0; i--)
            {
                ungetc(left->lexeme[i], stdin);
            }

            advance();

            retp = or_expr();
        }
    }
    else
    {
        retp = or_expr();
    }

    return retp;
}

//or_expr          := xor_expr or_expr_tail
BTNode *or_expr(void) {
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

//or_expr_tail     := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{
    if (match(OR))
    {
        BTNode *node = makeNode(OR, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else
    {
        return left;
    }
}

//xor_expr         := and_expr xor_expr_tail
BTNode *xor_expr(void)
{
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{
    if (match(XOR))
    {
        BTNode *node = makeNode(XOR, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return xor_expr_tail(node);
    }
    else
    {
        return left;
    }
}

//and_expr         := addsub_expr and_expr_tail
BTNode *and_expr(void)
{
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

//and_expr_tail    := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{
    if (match(AND))
    {
        BTNode *node = makeNode(AND, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return and_expr_tail(node);
    }
    else
    {
        return left;
    }
}

//addsub_expr      := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void)
{
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

//addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{
    if (match(ADDSUB))
    {
        BTNode *node = makeNode(ADDSUB, getLexeme());
        node->left = left;
        advance();
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else
    {
        return left;
    }
}

//muldiv_expr      := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void)
{
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

//muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{
    if (match(MULDIV))
    {
        BTNode *node = makeNode(MULDIV, getLexeme());
        node->left = left;
        advance();
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else
    {
        return left;
    }
}

//unary_expr       := ADDSUB unary_expr | factor
BTNode *unary_expr(void)
{
    if (match(ADDSUB))
    {
        BTNode *node = NULL;
        if (strcmp(getLexeme(), "+") == 0)
        {
            advance();
            node = unary_expr();
        }
        else if (strcmp(getLexeme(), "-") == 0)
        {
            node = makeNode(ADDSUB, "-");
            node->left = makeNode(INT, "0");
            advance();
            node->right = unary_expr();
        }
        else
            error(UNKNOWN);

        return node;
    }
    else
    {
        return factor();
    }
}

//factor           := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN | (ID INCDEC ??)
BTNode *factor(void)
{
    BTNode *rept = NULL;
    if (match(INT))
    {
        rept = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        if (is_new_var(getLexeme()))
            error(SYNTAXERR);
        rept = makeNode(ID, getLexeme());
        advance();
    }
    else if (match(INCDEC))
    {
        if (strcmp(getLexeme(), "++") == 0)
        {
            rept = makeNode(INCDEC, "++");
            advance();
            if (match(ID))
            {
                rept->left = makeNode(ID, getLexeme());
                advance();
            }
            else
                error(SYNTAXERR);
        }
        else if (strcmp(getLexeme(), "--") == 0)
        {
            rept = makeNode(INCDEC, "--");
            advance();
            if (match(ID))
            {
                if (is_new_var(getLexeme()))
                    error(SYNTAXERR);
                rept->left = makeNode(ID, getLexeme());
                advance();
            }
            else
                error(SYNTAXERR);
        }
        else
        {
            error(UNDEFINED);
        }
    }
    else if (match(LPAREN))
    {
        advance();
        rept = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(SYNTAXERR);
    }
    else
    {
        error(SYNTAXERR);
    }

    return rept;
}


void err(ErrorType errorNum) {
    if (PRINTERR_ERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}
