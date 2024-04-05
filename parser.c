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
        printf("ID\n");
        left = makeNode(ID, getLexeme());
        advance();
        if (match(ASSIGN))
        {
            printf("ASSIGN\n");
            retp = makeNode(ASSIGN, getLexeme());
            retp->left = left;
            advance();
            retp->right = assign_expr();
        }
        else if (match(ADDSUB_ASSIGN))
        {
            printf("ADDSUB ASSIGN\n");
            retp = makeNode(ADDSUB_ASSIGN, getLexeme());
            retp->left = left;
            advance();
            retp->right = assign_expr();
        }
    }
    else
    {
        printf("or_expr");
        retp = or_expr();
    }

    return retp;
}

//or_expr          := xor_expr or_expr_tail
BTNode *or_expr(void)
{

}

//or_expr_tail     := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{

}

//xor_expr         := and_expr xor_expr_tail
BTNode *xor_expr(void)
{

}

//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{

}

//and_expr         := addsub_expr and_expr_tail
BTNode *and_expr(void)
{

}

//and_expr_tail    := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{

}

//addsub_expr      := muldiv_expr addsub_expr_tail
BTNode *addsub_expr(void)
{

}

//addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{

}

//muldiv_expr      := unary_expr muldiv_expr_tail
BTNode *muldiv_expr(void)
{

}

//muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{

}

//unary_expr       := ADDSUB unary_expr | factor
BTNode *unary_expr(void)
{

}

//factor           := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void)
{

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
            retp = makeNode(INCDEC, "++p");
        else
            retp = makeNode(INCDEC, "--p");

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
    if (PRINTERR) {
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
