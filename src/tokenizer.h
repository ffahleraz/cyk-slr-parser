#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"

#define MAX_TOKEN_STR_LEN 40
#define MAX_TOKEN_COUNT 1000
#define TERM_COUNT 52

typedef enum TokenType {

    /* Words */
    // Program Keywords
    PROGRAM, VAR, BEGIN, END, IF, THEN, ELSE, WHILE, DO, FOR, TO, DOWNTO,
    STEP, REPEAT, UNTIL, INPUT, OUTPUT, OF,

    // Data Types
    INTEGER, REAL, CHAR, ARRAY,

    // Logical Operators
    AND, OR,

    // Mathematical Operators
    MOD, DIV,

    /* Literals */
    // Values
    INTEGER_VAL, REAL_VAL, CHAR_VAL,

    // Identifier
    IDENTIFIER,

    /* Signs */
    // Grammatical
    COLON, SEMICOLON, DOT, COMMA, OPEN_ROUND_PAREN, CLOSE_ROUND_PAREN,
    OPEN_ANGLE_PAREN, CLOSE_ANGLE_PAREN,

    // Assignment & Mathematical Operators
    PLUS, MINUS, STAR, SLASH, COLON_EQUAL,

    // Conditional Operators
    LESS_THAN, LESS_THAN_EQUAL, GREATER_THAN, GREATER_THAN_EQUAL, EQUAL,
    NOT_EQUAL,

    // Range Identifier
    DOUBLE_DOT,

    // SLR
    TERMINATE_TOKEN,

    // Invalid Token
    INVALID

} TokenType;

typedef struct Token {
    TokenType type;
    char str[MAX_TOKEN_STR_LEN];
    int line;
    int col;
} Token;

/* External variables */
extern Token tokens[MAX_TOKEN_COUNT];
extern int token_count;

/* Global variables */
FILE *file;
char cbuff;
int curr_line;
int curr_col;
int prev_col;
Token curr_token;

/* Prototypes */
void OpenSourceFile(char *file_name);
void CloseSourceFile();

boolean IsAtEnd();

boolean IsBlank(char c);
boolean IsAlpha(char c);
boolean IsNum(char c);
boolean IsAlphaNum(char c);
boolean IsNumLiteral(char *str);

void ToLowerCase(char *str);

void Advance();
void Jump(int len);
void Retreat(int len);
boolean Match(char *literal);
void AdvanceToken();

void Tokenize(char *file_name);

void GetTokenName(char* term_name, int type);
int GetTokenType(char* term_name);

#endif
