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
    if (PRINTERR) {
        printf("=========new var detected!========\n");
    }
    return true;
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

    if (tok == INT)
        node->Placement = CONST;
    else if (tok == END || tok == ENDFILE)
        node->Placement = NOTDATA;
    else
        node->Placement = UNDEF;

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
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");

            generateAssembly(retp);

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
    if (PRINTERR) {
        printf("assign_expr\n");
    }
    BTNode *retp = NULL, *left = NULL;

    if (match(ID))
    {
        if (PRINTERR) {
            printf("ID\n");
        }
        left = makeNode(ID, getLexeme());
        advance();

        if (match(ASSIGN))
        {
            if (PRINTERR) {
                printf("ASSIGN\n");
            }
            retp = makeNode(ASSIGN, getLexeme());
            retp->left = left;
            advance();
            retp->right = assign_expr();
            if (PRINTERR) {
                printf("A\n");

                printf("%s\n%s\n", retp->left->lexeme, retp->right->lexeme);
            }
        }
        else if (match(ADDSUB_ASSIGN))
        {
            if (PRINTERR) {
                printf("ADDSUB ASSIGN\n");
            }
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

            if (PRINTERR) {
                printf("Start putting ANYTHING back to stream\n");
                printf("str=%s\nlen=%d\n", str, len);
            }

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

            if (PRINTERR) {
                printf("Start putting ID back to stream\n");
                printf("str=%s\nlen=%d\n", left->lexeme, len);
            }

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
    if (PRINTERR) {
        printf("or_expr\n");
    }
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

//or_expr_tail     := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{
    if (PRINTERR) {
        printf("or_expr_tail\n");
    }
    if (match(OR))
    {
        if (PRINTERR) {
            printf("OR\n");
        }
        BTNode *node = makeNode(OR, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else
    {
        if (PRINTERR) {
            printf("NiL\n");
            printf("Not or but %s\n", getLexeme());
        }
        return left;
    }
}

//xor_expr         := and_expr xor_expr_tail
BTNode *xor_expr(void)
{
    if (PRINTERR) {
        printf("xor_expr\n");
    }
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{
    if (PRINTERR) {
        printf("xor_expr_tail\n");
    }
    if (match(XOR))
    {
        if (PRINTERR) {
            printf("XOR\n");
        }
        BTNode *node = makeNode(XOR, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return xor_expr_tail(node);
    }
    else
    {
        if (PRINTERR) {
            printf("NiL\n");
            printf("Not xor but %s\n", getLexeme());
        }
        return left;
    }
}

//and_expr         := addsub_expr and_expr_tail
BTNode *and_expr(void)
{
    if (PRINTERR) {
        printf("and_expr\n");
    }
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

//and_expr_tail    := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{
    if (PRINTERR) {
        printf("and_expr_tail\n");
    }
    if (match(AND))
    {
        if (PRINTERR) {
            printf("AND\n");
        }
        BTNode *node = makeNode(AND, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return and_expr_tail(node);
    }
    else
    {
        if (PRINTERR) {
            printf("NiL\n");
            printf("Not and but %s\n", getLexeme());
        }
        return left;
    }
}

//addsub_expr      := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void)
{
    if (PRINTERR) {
        printf("addsub_expr\n");
    }
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

//addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{
    if (PRINTERR) {
        printf("addsub_expr_tail\n");
    }
    if (match(ADDSUB))
    {
        if (PRINTERR) {
            printf("ADDSUB\n");
        }
        BTNode *node = makeNode(ADDSUB, getLexeme());
        node->left = left;
        advance();
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else
    {
        if (PRINTERR) {
            printf("NiL\n");
            printf("Not addsub but %s\n", getLexeme());
        }
        return left;
    }
}

//muldiv_expr      := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void)
{
    if (PRINTERR) {
        printf("muldiv_expr\n");
    }
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

//muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{
    if (PRINTERR) {
        printf("muldiv_expr_tail\n");
    }
    if (match(MULDIV))
    {
        if (PRINTERR) {
            printf("MULDIV\n");
        }
        BTNode *node = makeNode(MULDIV, getLexeme());
        node->left = left;
        advance();
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else
    {
        if (PRINTERR) {
            printf("NiL\n");
            printf("Not muldiv but %s\n", getLexeme());
        }
        return left;
    }
}

//unary_expr       := ADDSUB unary_expr | factor
BTNode *unary_expr(void)
{
    if (PRINTERR) {
        printf("unary_expr\n");
    }
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
    if (PRINTERR) {
        printf("factor\n");
    }
    if (match(INT))
    {
        if (PRINTERR) {
            printf("Get INT!\n");
        }
        rept = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        if (PRINTERR) {
            printf("Get ID! ID=%s\n", getLexeme());
        }
        if (is_new_var(getLexeme()))
            error(SYNTAXERR);
        rept = makeNode(ID, getLexeme());
        advance();
    }
    else if (match(INCDEC))
    {
        if (strcmp(getLexeme(), "++") == 0)
        {
            rept = makeNode(INCDEC, "++b");
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
            rept = makeNode(INCDEC, "--b");
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






// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  | 
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *old_factor(void) {
    BTNode *retp = NULL, *left = NULL;

    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(INCDEC)) {
        if (strcmp(getLexeme(), "++") == 0)
            retp = makeNode(INCDEC, "++b");
        else
            retp = makeNode(INCDEC, "--b");

        advance();
        if (match(ID)) {
            left = makeNode(ID, getLexeme());
            retp->left = left;
            advance();
        } else {
            error(SYNTAXERR);
        }
    } else if (match(ID)) {
        left = makeNode(ID, getLexeme());
        advance();
        if (!match(ASSIGN)) {
            if(match(ADDSUB_ASSIGN))
            {
                retp = makeNode(ADDSUB_ASSIGN, getLexeme());
                advance();
                retp->left = left;
                retp->right = expr();
            }
            else if (match(INCDEC))
            {
                if (strcmp(getLexeme(), "++") == 0)
                    retp = makeNode(INCDEC, "++a");
                else
                    retp = makeNode(INCDEC, "--a");
                retp->left = left;
                advance();
            }
            else
            {
                retp = left;
            }
        } else {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
    } else if (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        retp->left = makeNode(INT, "0");
        advance();
        if (match(INT)) {
            retp->right = makeNode(INT, getLexeme());
            advance();
        } else if (match(ID)) {
            retp->right = makeNode(ID, getLexeme());
            advance();
        } else if (match(LPAREN)) {
            advance();
            retp->right = expr();
            if (match(RPAREN))
                advance();
            else
                error(MISPAREN);
        } else {
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}

// term := factor term_tail
BTNode *term(void) {
    BTNode *node = old_factor();
    return term_tail(node);
}

// term_tail := MULDIV factor term_tail | NiL
BTNode *term_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = old_factor();
        return term_tail(node);
    } else {
        return left;
    }
}

// expr := term expr_tail
BTNode *expr(void) {
    BTNode *node = term();
    return expr_tail(node);
}

// expr_tail := ADDSUB term expr_tail | NiL
BTNode *expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = term();
        return expr_tail(node);
    }
    else if (match(OR)) {
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = term();
        return expr_tail(node);
    }
    else {
        return left;
    }
}

// statement := ENDFILE | END | expr END
void old_statement(void) {
    BTNode *retp = NULL;

    if (match(ENDFILE)) {
        exit(0);
    } else if (match(END)) {
        printf(">> ");
        advance();
    } else {
        retp = expr();
        if (match(END)) {
            printf("%d\n", evaluateTree(retp));
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
