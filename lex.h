#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,
    ADDSUB,
    INCDEC,
    MULDIV,
    ASSIGN,
    LPAREN, RPAREN,

    AND, OR, XOR

} TokenSet;

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__
