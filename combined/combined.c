/////////////////////////////////////////////////////////////////////////////////////////////////
//lex.h
#define MAXLEN 256

typedef enum {
    UNKNOWN, END, ENDFILE,
    INT, ID,
    ADDSUB,
    INCDEC,
    MULDIV,
    ASSIGN,
    ADDSUB_ASSIGN,
    LPAREN, RPAREN,

    AND, OR, XOR

} TokenSet;

extern int match(TokenSet token);
extern void advance(void);
extern char *getLexeme(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
//parser.h
#include <stdbool.h>
#define TBLSIZE 64

#define PRINTERR 0
#define PRINTERR_ERR 0

#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    printf("EXIT 1\n");                      \
    err(errorNum); \
}

typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    char location[32];
    struct _Node *left;
    struct _Node *right;
} BTNode;

extern Symbol table[TBLSIZE];

extern void initTable(void);

extern int getmem(char *str);

extern int getval(char *str);

extern int setval(char *str, int val);

extern BTNode *makeNode(TokenSet tok, const char *lexe);

extern void freeTree(BTNode *root);

extern void statement(void);
extern BTNode *assign_expr(void);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *unary_expr(void);
extern BTNode *factor(void);

extern void err(ErrorType errorNum);

extern bool is_new_var(char *str);

/////////////////////////////////////////////////////////////////////////////////////////////////
//codeGen.h

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

/////////////////////////////////////////////////////////////////////////////////////////////////
//lex.c

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c;
        c = fgetc(stdin);

        if (c == lexeme[0])
        {
            lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        }
        else if (c == '=')
        {
            lexeme[1] = '=';
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }
        else
        {
            lexeme[1] = '\0';
            ungetc(c, stdin);
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c) || c == '_') {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ( (isalpha(c) || isdigit(c) || c == '_') && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } else if (c == '&') {
        lexeme[0] = '&';
        lexeme[1] = '\0';
        return AND;
    } else if (c == '|') {
        lexeme[0] = '|';
        lexeme[1] = '\0';
        return OR;
    } else if (c == '^') {
        lexeme[0] = '^';
        lexeme[1] = '\0';
        return XOR;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//parser.c

#include <stdlib.h>

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
            evaluateTree(retp);
            freeTree(retp);
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

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

BTNode *or_expr(void) {
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

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

BTNode *xor_expr(void)
{
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

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

BTNode *and_expr(void)
{
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

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

BTNode *addsub_expr(void)
{
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

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

BTNode *muldiv_expr(void)
{
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

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
    exit(0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//codeGen.c

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

bool hasIDInTree(BTNode* node) {
    if (node == NULL)
        return false;
    else if (node->data == ID)
        return true;
    else
        return hasIDInTree(node->left) || hasIDInTree(node->right);
}

ReturnType evaluateTree(BTNode *root) {
    ReturnType retval, lv, rv;
    retval.rtvl = 0;
    lv.rtvl = 0;
    rv.rtvl = 0;

    int rx = 0;
    int ry = 0;
    char command[4];

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
                if (rv.type == VAR) {
                    ry = findRegAvailableAndUse();
                    printf("MOV r%d [%d]\n", ry, rv.value);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else if (rv.type == CONST) {
                    ry = findRegAvailableAndUse();
                    printf("MOV r%d %d\n", ry, rv.value);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else if (rv.type == REG) {
                    ry = rv.value;
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), ry);
                } else {
                    printf("Warning: rv without type\n");
                }
                retval.value = ry;

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
                    strcpy(command, "MUL");
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv.rtvl == 0) {
                        if (!hasIDInTree(root->right)) {
                            error(DIVZERO);
                        } else {
                            retval.rtvl = -1;
                        }
                    } else {
                        retval.rtvl = lv.rtvl / rv.rtvl;
                    }

                    strcpy(command, "DIV");
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
            case OR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl | rv.rtvl;
                strcpy(command, "OR");

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
            case AND:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl & rv.rtvl;

                strcpy(command, "AND");

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
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                retval.rtvl = lv.rtvl ^ rv.rtvl;

                strcpy(command, "XOR");

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
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if(strcmp(root->lexeme, "+=") == 0) {
                    retval.rtvl = setval(root->left->lexeme, lv.rtvl + rv.rtvl);
                    strcpy(command, "ADD");
                } else {
                    retval.rtvl = setval(root->left->lexeme, lv.rtvl - rv.rtvl);
                    strcpy(command, "SUB");
                }

                retval.type = REG;
                rx = findRegAvailableAndUse();
                if (rv.type == VAR) {
                    ry = findRegAvailableAndUse();
                    printf("MOV r%d [%d]\n", rx, getmem(root->left->lexeme));
                    printf("MOV r%d [%d]\n", ry, rv.value);
                    printf("%s r%d r%d\n", command, rx, ry);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), rx);
                    isRegAvailable[ry] = true;
                } else if (rv.type == CONST) {
                    ry = findRegAvailableAndUse();
                    printf("MOV r%d [%d]\n", rx, getmem(root->left->lexeme));
                    printf("MOV r%d %d\n", ry, rv.value);
                    printf("%s r%d r%d\n", command, rx, ry);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), rx);
                    isRegAvailable[ry] = true;
                } else if (rv.type == REG) {
                    ry = rv.value;
                    printf("MOV r%d [%d]\n", rx, getmem(root->left->lexeme));
                    printf("%s r%d r%d\n", command, rx, ry);
                    printf("MOV [%d] r%d\n", getmem(root->left->lexeme), rx);
                    isRegAvailable[ry] = true;
                } else {
                    printf("Warning: rv without type\n");
                }
                retval.value = rx;

                break;
            case INCDEC:
                if (strcmp(root->lexeme, "++") == 0) {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl + 1);
                    strcpy(root->lexeme, "++");
                    strcpy(command, "ADD");
                } else {
                    retval.rtvl = setval(root->left->lexeme, evaluateTree(root->left).rtvl - 1);
                    strcpy(root->lexeme, "--");
                    strcpy(command, "SUB");
                }

                retval.type = REG;
                rx = findRegAvailableAndUse();

                ry = findRegAvailableAndUse();
                printf("MOV r%d [%d]\n", rx, getmem(root->left->lexeme));
                printf("MOV r%d %d\n", ry, 1);
                printf("%s r%d r%d\n", command, rx, ry);
                printf("MOV [%d] r%d\n", getmem(root->left->lexeme), rx);
                isRegAvailable[ry] = true;

                retval.value = rx;

                break;
            default:
                retval.rtvl = 0;
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


/////////////////////////////////////////////////////////////////////////////////////////////////
//main.c

int main() {
    initTable();

    while (1) {
        initReg();
        statement();
    }
    return 0;
}
