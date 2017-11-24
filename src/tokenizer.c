#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "tokenizer.h"

/* External variables */
Token tokens[MAX_TOKEN_COUNT];
int token_count;

/* Global variables */
FILE *file;
char cbuff;
int curr_line;
int curr_col;
int prev_col;
Token curr_token;

void OpenSourceFile(char *file_name) {

    file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error: file not found\n");
        exit(1);
    }

}

void CloseSourceFile() {

    fclose(file);

}

boolean IsAtEnd() {

    return (cbuff == EOF);

}

boolean IsBlank(char c) {

    return (c == ' ' || c == '\n');

}

boolean IsAlpha(char c) {

    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));

}

boolean IsNum(char c) {

    return (c >= '0' && c <= '9');

}

boolean IsAlphaNum(char c) {

    return (IsAlpha(c) || IsNum(c));

}

boolean IsNumLiteral(char *str) {

    int len = strlen(str);
    for (int i = 0; i < len; i ++) {
        if (!IsNum(str[i])) {
            return false;
        }
    }
    return true;

}

void ToLowerCase(char *str) {

    for (int i = 0; i < strlen(str); i ++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 'a' - 'A';
        }
    }

}

void Advance() {

    do {
        cbuff = fgetc(file);
        curr_col ++;
        if (cbuff == '\n') {
            prev_col = curr_col;
            curr_col = 0;
            curr_line ++;
        }
    } while (IsBlank(cbuff));

}

void Jump(int len) {

    fseek(file, (len - 1) * sizeof(char), SEEK_CUR);
    cbuff = fgetc(file);
    curr_col ++;
    if (cbuff == '\n') {
        prev_col = curr_col;
        curr_col = 0;
        curr_line ++;
    }

}

void Retreat(int len) {

    int curr_pos = ftell(file);
    if (curr_pos >= (len + 1) * sizeof(char)) {
        if (cbuff == '\n') {
            curr_col = prev_col;
            curr_line --;
        }
        for (int i = 0; i < len; i ++) {
            fseek(file, -2 * sizeof(char), SEEK_CUR);
            cbuff = fgetc(file);
            curr_col --;
        }
    } else {
        printf("Error: can't backtrack beyond start of file\n");
    }

}

boolean Match(char *str) {

    int len = strlen(str);
    for (int i = 0; i < len; i ++) {
        if (cbuff == EOF || cbuff != str[i]) {
            Retreat(i);
            return false;
        }
        Jump(1);
    }
    Retreat(len);
    return true;

}

void AdvanceToken() {

    curr_token.line = curr_line;
    curr_token.col = curr_col;

    char curr_token_str[MAX_TOKEN_STR_LEN];

    // Signs
    if (!IsAlphaNum(cbuff)) {

        // Two char signs
        if (Match(":=")) {
            strcpy(curr_token_str, ":=");
            Jump(1);
            curr_token.type = COLON_EQUAL;

        } else if (Match("<=")) {
            strcpy(curr_token_str, "<=");
            Jump(1);
            curr_token.type = LESS_THAN_EQUAL;

        } else if (Match(">=")) {
            strcpy(curr_token_str, ">=");
            Jump(1);
            curr_token.type = GREATER_THAN_EQUAL;

        } else if (Match("<>")) {
            strcpy(curr_token_str, "<>");
            Jump(1);
            curr_token.type = NOT_EQUAL;

        } else if (Match("..")) {
            strcpy(curr_token_str, "..");
            Jump(1);
            curr_token.type = DOUBLE_DOT;

        // Char literal or single quote
        } else if (cbuff == '\'') {
            curr_token_str[0] = cbuff;
            Jump(1);
            curr_token_str[1] = cbuff;
            Jump(1);
            if (cbuff == '\'') {
                curr_token_str[2] = cbuff;
                curr_token_str[3] = '\0';
                curr_token.type = CHAR_VAL;
            } else {
                Retreat(2);
                curr_token_str[1] = '\0';
                curr_token.type = INVALID;
            }

        // Single char tokens except single quote
        } else {

            curr_token_str[0] = cbuff;
            curr_token_str[1] = '\0';

            if (cbuff == ':') {
                curr_token.type = COLON;

            } else if (cbuff == ';') {
                curr_token.type = SEMICOLON;

            } else if (cbuff == '.') {
                curr_token.type = DOT;

            } else if (cbuff == ',') {
                curr_token.type = COMMA;

            } else if (cbuff == '(') {
                curr_token.type = OPEN_ROUND_PAREN;

            } else if (cbuff == ')') {
                curr_token.type = CLOSE_ROUND_PAREN;

            } else if (cbuff == '[') {
                curr_token.type = OPEN_ANGLE_PAREN;

            } else if (cbuff == ']') {
                curr_token.type = CLOSE_ANGLE_PAREN;

            } else if (cbuff == '+') {
                curr_token.type = PLUS;

            } else if (cbuff == '-') {
                curr_token.type = MINUS;

            } else if (cbuff == '*') {
                curr_token.type = STAR;

            } else if (cbuff == '/') {
                curr_token.type = SLASH;

            } else if (cbuff == '<') {
                curr_token.type = LESS_THAN;

            } else if (cbuff == '>') {
                curr_token.type = GREATER_THAN;

            } else if (cbuff == '=') {
                curr_token.type = EQUAL;

            } else {
                curr_token.type = INVALID;
            }
        }

    // Words or literals except char literal
    } else {

        // Integer or real literal
        if (IsNum(cbuff)) {

            int i = 0;
            while(IsNum(cbuff)) {
                curr_token_str[i] = cbuff;
                Jump(1);
                i ++;
            }

            if (cbuff == '.') {

                curr_token_str[i] = cbuff;
                Jump(1);
                if (IsNum(cbuff)) {
                    int j = 1;
                    while(IsNum(cbuff)) {
                        curr_token_str[i + j] = cbuff;
                        Jump(1);
                        j ++;
                    }
                    curr_token_str[i + j] = '\0';
                    Retreat(1);
                    curr_token.type = REAL_VAL;
                } else {
                    curr_token_str[i] = '\0';
                    Retreat(2);
                    curr_token.type = INTEGER_VAL;
                }

            } else {
                curr_token_str[i] = '\0';
                Retreat(1);
                curr_token.type = INTEGER_VAL;
            }

        // Words or identifier
        } else {

            int i = 0;
            while(IsAlphaNum(cbuff)) {
                curr_token_str[i] = cbuff;
                Jump(1);
                i ++;
            }
            curr_token_str[i] = '\0';
            Retreat(1);

            // Convert to lowercase
            ToLowerCase(curr_token_str);

            if (strcmp(curr_token_str, "program") == 0) {
                curr_token.type = PROGRAM;

            } else if (strcmp(curr_token_str, "var") == 0) {
                curr_token.type = VAR;

            } else if (strcmp(curr_token_str, "begin") == 0) {
                curr_token.type = BEGIN;

            } else if (strcmp(curr_token_str, "end") == 0) {
                curr_token.type = END;

            } else if (strcmp(curr_token_str, "if") == 0) {
                curr_token.type = IF;

            } else if (strcmp(curr_token_str, "then") == 0) {
                curr_token.type = THEN;

            } else if (strcmp(curr_token_str, "else") == 0) {
                curr_token.type = ELSE;

            } else if (strcmp(curr_token_str, "while") == 0) {
                curr_token.type = WHILE;

            } else if (strcmp(curr_token_str, "do") == 0) {
                curr_token.type = DO;

            } else if (strcmp(curr_token_str, "for") == 0) {
                curr_token.type = FOR;

            } else if (strcmp(curr_token_str, "to") == 0) {
                curr_token.type = TO;

            } else if (strcmp(curr_token_str, "downto") == 0) {
                curr_token.type = DOWNTO;

            } else if (strcmp(curr_token_str, "step") == 0) {
                curr_token.type = STEP;

            } else if (strcmp(curr_token_str, "repeat") == 0) {
                curr_token.type = REPEAT;

            } else if (strcmp(curr_token_str, "until") == 0) {
                curr_token.type = UNTIL;

            } else if (strcmp(curr_token_str, "input") == 0) {
                curr_token.type = INPUT;

            } else if (strcmp(curr_token_str, "output") == 0) {
                curr_token.type = OUTPUT;

            } else if (strcmp(curr_token_str, "of") == 0) {
                curr_token.type = OF;

            } else if (strcmp(curr_token_str, "integer") == 0) {
                curr_token.type = INTEGER;

            } else if (strcmp(curr_token_str, "real") == 0) {
                curr_token.type = REAL;

            } else if (strcmp(curr_token_str, "char") == 0) {
                curr_token.type = CHAR;

            } else if (strcmp(curr_token_str, "array") == 0) {
                curr_token.type = ARRAY;

            } else if (strcmp(curr_token_str, "and") == 0) {
                curr_token.type = AND;

            } else if (strcmp(curr_token_str, "or") == 0) {
                curr_token.type = OR;

            } else if (strcmp(curr_token_str, "mod") == 0) {
                curr_token.type = MOD;

            } else if (strcmp(curr_token_str, "div") == 0) {
                curr_token.type = DIV;

            } else {
                curr_token.type = IDENTIFIER;
            }
        }
    }

    strcpy(curr_token.str, curr_token_str);
    Advance();

}

void Tokenize(char *file_name) {

    // Open file and start reading
    OpenSourceFile(file_name);
    Advance();

    curr_line = 1;
    curr_col = 1;
    token_count = 0;

    // Main loop
    while (cbuff != EOF) {

        AdvanceToken();
        tokens[token_count] = curr_token;
        token_count ++;

    }

    CloseSourceFile();

}

void GetTokenName(char* term_name, int type) {

    switch (type) {
        case PROGRAM:
            strcpy(term_name, "PROGRAM");
            break;
        case VAR:
            strcpy(term_name, "VAR");
            break;
        case BEGIN:
            strcpy(term_name, "BEGIN");
            break;
        case END:
            strcpy(term_name, "END");
            break;
        case IF:
            strcpy(term_name, "IF");
            break;
        case THEN:
            strcpy(term_name, "THEN");
            break;
        case ELSE:
            strcpy(term_name, "ELSE");
            break;
        case WHILE:
            strcpy(term_name, "WHILE");
            break;
        case DO:
            strcpy(term_name, "DO");
            break;
        case FOR:
            strcpy(term_name, "FOR");
            break;
        case TO:
            strcpy(term_name, "TO");
            break;
        case DOWNTO:
            strcpy(term_name, "DOWNTO");
            break;
        case STEP:
            strcpy(term_name, "STEP");
            break;
        case REPEAT:
            strcpy(term_name, "REPEAT");
            break;
        case UNTIL:
            strcpy(term_name, "UNTIL");
            break;
        case INPUT:
            strcpy(term_name, "INPUT");
            break;
        case OUTPUT:
            strcpy(term_name, "OUTPUT");
            break;
        case OF:
            strcpy(term_name, "OF");
            break;
        case INTEGER:
            strcpy(term_name, "INTEGER");
            break;
        case REAL:
            strcpy(term_name, "REAL");
            break;
        case CHAR:
            strcpy(term_name, "CHAR");
            break;
        case ARRAY:
            strcpy(term_name, "ARRAY");
            break;
        case AND:
            strcpy(term_name, "AND");
            break;
        case OR:
            strcpy(term_name, "OR");
            break;
        case MOD:
            strcpy(term_name, "MOD");
            break;
        case DIV:
            strcpy(term_name, "DIV");
            break;
        case INTEGER_VAL:
            strcpy(term_name, "INTEGER_VAL");
            break;
        case REAL_VAL:
            strcpy(term_name, "REAL_VAL");
            break;
        case CHAR_VAL:
            strcpy(term_name, "CHAR_VAL");
            break;
        case IDENTIFIER:
            strcpy(term_name, "IDENTIFIER");
            break;
        case INVALID:
            strcpy(term_name, "INVALID");
            break;
        case COLON:
            strcpy(term_name, "COLON");
            break;
        case SEMICOLON:
            strcpy(term_name, "SEMICOLON");
            break;
        case DOT:
            strcpy(term_name, "DOT");
            break;
        case COMMA:
            strcpy(term_name, "COMMA");
            break;
        case OPEN_ROUND_PAREN:
            strcpy(term_name, "OPEN_ROUND_PAREN");
            break;
        case CLOSE_ROUND_PAREN:
            strcpy(term_name, "CLOSE_ROUND_PAREN");
            break;
        case OPEN_ANGLE_PAREN:
            strcpy(term_name, "OPEN_ANGLE_PAREN");
            break;
        case CLOSE_ANGLE_PAREN:
            strcpy(term_name, "CLOSE_ANGLE_PAREN");
            break;
        case PLUS:
            strcpy(term_name, "PLUS");
            break;
        case MINUS:
            strcpy(term_name, "MINUS");
            break;
        case STAR:
            strcpy(term_name, "STAR");
            break;
        case SLASH:
            strcpy(term_name, "SLASH");
            break;
        case COLON_EQUAL:
            strcpy(term_name, "COLON_EQUAL");
            break;
        case LESS_THAN:
            strcpy(term_name, "LESS_THAN");
            break;
        case LESS_THAN_EQUAL:
            strcpy(term_name, "LESS_THAN_EQUAL");
            break;
        case GREATER_THAN:
            strcpy(term_name, "GREATER_THAN");
            break;
        case GREATER_THAN_EQUAL:
            strcpy(term_name, "GREATER_THAN_EQUAL");
            break;
        case EQUAL:
            strcpy(term_name, "EQUAL");
            break;
        case NOT_EQUAL:
            strcpy(term_name, "NOT_EQUAL");
            break;
        case DOUBLE_DOT:
            strcpy(term_name, "DOUBLE_DOT");
            break;
        default:
            strcpy(term_name, "INVALID");
            break;
    }

}

int GetTokenType(char* term_name) {

    if (strcmp(term_name, "PROGRAM") == 0) {
        return PROGRAM;

    } else if (strcmp(term_name, "VAR") == 0) {
        return VAR;

    } else if (strcmp(term_name, "BEGIN") == 0) {
        return BEGIN;

    } else if (strcmp(term_name, "END") == 0) {
        return END;

    } else if (strcmp(term_name, "IF") == 0) {
        return IF;

    } else if (strcmp(term_name, "THEN") == 0) {
        return THEN;

    } else if (strcmp(term_name, "ELSE") == 0) {
        return ELSE;

    } else if (strcmp(term_name, "WHILE") == 0) {
        return WHILE;

    } else if (strcmp(term_name, "DO") == 0) {
        return DO;

    } else if (strcmp(term_name, "FOR") == 0) {
        return FOR;

    } else if (strcmp(term_name, "TO") == 0) {
        return TO;

    } else if (strcmp(term_name, "DOWNTO") == 0) {
        return DOWNTO;

    } else if (strcmp(term_name, "STEP") == 0) {
        return STEP;

    } else if (strcmp(term_name, "REPEAT") == 0) {
        return REPEAT;

    } else if (strcmp(term_name, "UNTIL") == 0) {
        return UNTIL;

    } else if (strcmp(term_name, "INPUT") == 0) {
        return INPUT;

    } else if (strcmp(term_name, "OUTPUT") == 0) {
        return OUTPUT;

    } else if (strcmp(term_name, "OF") == 0) {
        return OF;

    } else if (strcmp(term_name, "INTEGER") == 0) {
        return INTEGER;

    } else if (strcmp(term_name, "REAL") == 0) {
        return REAL;

    } else if (strcmp(term_name, "CHAR") == 0) {
        return CHAR;

    } else if (strcmp(term_name, "ARRAY") == 0) {
        return ARRAY;

    } else if (strcmp(term_name, "AND") == 0) {
        return AND;

    } else if (strcmp(term_name, "OR") == 0) {
        return OR;

    } else if (strcmp(term_name, "MOD") == 0) {
        return MOD;

    } else if (strcmp(term_name, "DIV") == 0) {
        return DIV;

    } else if (strcmp(term_name, "INTEGER_VAL") == 0) {
        return INTEGER_VAL;

    } else if (strcmp(term_name, "REAL_VAL") == 0) {
        return REAL_VAL;

    } else if (strcmp(term_name, "CHAR_VAL") == 0) {
        return CHAR_VAL;

    } else if (strcmp(term_name, "IDENTIFIER") == 0) {
        return IDENTIFIER;

    } else if (strcmp(term_name, "INVALID") == 0) {
        return INVALID;

    } else if (strcmp(term_name, "COLON") == 0) {
        return COLON;

    } else if (strcmp(term_name, "SEMICOLON") == 0) {
        return SEMICOLON;

    } else if (strcmp(term_name, "DOT") == 0) {
        return DOT;

    } else if (strcmp(term_name, "COMMA") == 0) {
        return COMMA;

    } else if (strcmp(term_name, "OPEN_ROUND_PAREN") == 0) {
        return OPEN_ROUND_PAREN;

    } else if (strcmp(term_name, "CLOSE_ROUND_PAREN") == 0) {
        return CLOSE_ROUND_PAREN;

    } else if (strcmp(term_name, "OPEN_ANGLE_PAREN") == 0) {
        return OPEN_ANGLE_PAREN;

    } else if (strcmp(term_name, "CLOSE_ANGLE_PAREN") == 0) {
        return CLOSE_ANGLE_PAREN;

    } else if (strcmp(term_name, "PLUS") == 0) {
        return PLUS;

    } else if (strcmp(term_name, "MINUS") == 0) {
        return MINUS;

    } else if (strcmp(term_name, "STAR") == 0) {
        return STAR;

    } else if (strcmp(term_name, "SLASH") == 0) {
        return SLASH;

    } else if (strcmp(term_name, "COLON_EQUAL") == 0) {
        return COLON_EQUAL;

    } else if (strcmp(term_name, "LESS_THAN") == 0) {
        return LESS_THAN;

    } else if (strcmp(term_name, "LESS_THAN_EQUAL") == 0) {
        return LESS_THAN_EQUAL;

    } else if (strcmp(term_name, "GREATER_THAN") == 0) {
        return GREATER_THAN;

    } else if (strcmp(term_name, "GREATER_THAN_EQUAL") == 0) {
        return GREATER_THAN_EQUAL;

    } else if (strcmp(term_name, "EQUAL") == 0) {
        return EQUAL;

    } else if (strcmp(term_name, "NOT_EQUAL") == 0) {
        return NOT_EQUAL;

    } else if (strcmp(term_name, "DOUBLE_DOT") == 0) {
        return DOUBLE_DOT;

    } else {
        return -1;
    }

}
