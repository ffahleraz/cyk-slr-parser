#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "tokenizer.h"
#include "cyk_parser.h"

/* Global variables */
char variables[MAX_VAR_COUNT][MAX_NAME_LEN];
int variable_count;

int start_variable_id;

TermProduction term_prods[MAX_TERM_PROD_COUNT];
int term_prod_count;

VarProduction var_prods[MAX_VAR_PROD_COUNT];
int var_prod_count;

// File
FILE *file;
char *ret;
char lbuff[MAX_LINE_LEN];

// CYK
CYKCell **cyk_table;
int cyk_table_dim;

/* CYK */
void OpenCNFFile(char *file_name) {

    file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Error: file not found\n");
        exit(1);
    }

}

void CloseCNFFile() {

    fclose(file);

}

void AdvanceLine() {

    ret = fgets(lbuff, MAX_LINE_LEN, file);

}

void IgnoreBlankLine() {

    while (IsAtBlankLine()) {
        AdvanceLine();
    }

}

boolean IsAtBlankLine() {

    char temp[MAX_NAME_LEN];
    return (lbuff[0] == '#' || sscanf(lbuff, " %s", temp) == EOF);

}

boolean IsAtEndLine() {

    return (ret == NULL);

}

void ReadVariables() {

    variable_count = 0;

    IgnoreBlankLine();
    while (!IsAtBlankLine() && !IsAtEndLine() && lbuff[0] != '%') {
        sscanf(lbuff, "%s", variables[variable_count]);
        variable_count ++;
        AdvanceLine();
        IgnoreBlankLine();
    }
    AdvanceLine();

}

void ReadStartVariable() {

    char start_variable_name[MAX_NAME_LEN];

    IgnoreBlankLine();
    sscanf(lbuff, "%s", start_variable_name);
    start_variable_id = GetVariableId(start_variable_name);
    AdvanceLine();
    while (IsAtBlankLine() && lbuff[0] != '%') {
        AdvanceLine();
    }
    AdvanceLine();

}

void ReadTerminalProductions() {

    char source_name[MAX_NAME_LEN];
    char term_name[MAX_NAME_LEN];

    term_prod_count = 0;

    IgnoreBlankLine();
    while (!IsAtBlankLine() && !IsAtEndLine() && lbuff[0] != '%') {
        sscanf(lbuff, "%s %s", source_name, term_name);
        term_prods[term_prod_count].source_id = GetVariableId(source_name);
        term_prods[term_prod_count].term_id = GetTerminalId(term_name);
        term_prod_count ++;
        AdvanceLine();
        IgnoreBlankLine();
    }
    AdvanceLine();

}

void ReadVariableProductions() {

    char source_name[MAX_NAME_LEN];
    char prod1_name[MAX_NAME_LEN];
    char prod2_name[MAX_NAME_LEN];

    var_prod_count = 0;

    IgnoreBlankLine();
    while (!IsAtBlankLine() && !IsAtEndLine()  && lbuff[0] != '%') {
        sscanf(lbuff, "%s %s %s", source_name, prod1_name, prod2_name);
        var_prods[var_prod_count].source_id = GetVariableId(source_name);
        var_prods[var_prod_count].prod1_id = GetVariableId(prod1_name);
        var_prods[var_prod_count].prod2_id = GetVariableId(prod2_name);
        var_prod_count ++;
        AdvanceLine();
        IgnoreBlankLine();
    }
    AdvanceLine();

}

boolean VarIsInCell(int row, int col, int var_id) {

    for (int k = 0; k < cyk_table[row][col].var_count; k ++) {
        if (cyk_table[row][col].vars[k] == var_id) {
            return true;
        }
    }
    return false;

}

void AddReducedTermTo(int row, int col, int term_id) {

    for (int k = 0; k < term_prod_count; k ++) {
        if (term_prods[k].term_id == term_id && !VarIsInCell(row, col, term_prods[k].source_id)) {
            cyk_table[row][col].vars[cyk_table[row][col].var_count] = term_prods[k].source_id;
            cyk_table[row][col].var_count ++;
        }
    }

}

void AddReducedVarsTo(int row, int col, int var1_id, int var2_id) {

    for (int k = 0; k < var_prod_count; k ++) {
        if (var_prods[k].prod1_id == var1_id && var_prods[k].prod2_id == var2_id && !VarIsInCell(row, col, var_prods[k].source_id)) {
            cyk_table[row][col].vars[cyk_table[row][col].var_count] = var_prods[k].source_id;
            cyk_table[row][col].var_count ++;
        }
    }

}

void ParseCell(int row, int col) {

    // printf("(%d,%d)\n", row, col);

    for (int k = 0; k < row; k ++) {
        // printf("(%d,%d) (%d,%d)\n", k, col, (row - k - 1), (col + k + 1));
        for (int l = 0; l < cyk_table[k][col].var_count; l ++) {
            for (int m = 0; m < cyk_table[row - k - 1][col + k + 1].var_count; m ++) {
                // printf("%d\n", cyk_table[k][col].var_count);
                AddReducedVarsTo(row, col, cyk_table[k][col].vars[l], cyk_table[row - k - 1][col + k + 1].vars[m]);
                // printf("%d %d\n", cyk_table[k][col].vars[l], cyk_table[row - k - 1][col + k + 1].vars[m]);
            }
        }
    }

}

void InitCYKTable(int dim) {

    cyk_table_dim = dim;
    cyk_table = malloc(sizeof(CYKCell *) * dim);
    for (int row = 0; row < dim; row ++) {
        cyk_table[row] = malloc(sizeof(CYKCell) * (dim - row));
        for (int col = 0; col < (dim - row); col ++) {
            cyk_table[row][col].vars = malloc(sizeof(int) * variable_count);
            cyk_table[row][col].var_count = 0;
        }
    }

    // Initialize bottom row
    for (int col = 0; col < cyk_table_dim; col ++) {
        AddReducedTermTo(0, col, tokens[col].type);
    }

}

void DeallocCYKTable() {

    for (int row = 0; row < cyk_table_dim; row ++) {
        for (int col = 0; col < (cyk_table_dim - row); col ++) {
            free(cyk_table[row][col].vars);
        }
        free(cyk_table[row]);
    }
    free(cyk_table);

}

void PrintCYKTable() {

    // TODO: masih salah kayanya

    for (int row = 0; row < cyk_table_dim; row ++) {
        for (int col = 0; col < (cyk_table_dim - row); col ++) {

            int padding = 11;
            int ord;

            printf("[");

            if (cyk_table[row][col].var_count == 0) {

                printf("]");
                padding --;

            } else {

                ord = cyk_table[row][col].vars[0];
                if (ord == 0) padding --;
                while (ord > 0) {
                    padding --;
                    ord = ord / 10;
                }
                printf("%d", cyk_table[row][col].vars[0]);

                for (int k = 1; k < cyk_table[row][col].var_count; k ++) {
                    ord = cyk_table[row][col].vars[k];
                    while (ord > 0) {
                        padding --;
                        ord = ord / 10;
                    }
                    printf(",%d", cyk_table[row][col].vars[k]);
                    padding --;
                }

                printf("]");
                padding --;

            }

            for (int i = 0; i < padding; i ++) {
                printf(" ");
            }

        }
        printf("\n");
    }

}

void CYK(char *file_name) {

    // Open CNF file and start reading
    OpenCNFFile(file_name);
    AdvanceLine();

    // Read and store all variables, terminals, and productions
    ReadVariables();
    ReadStartVariable();
    ReadTerminalProductions();
    ReadVariableProductions();

    // DEBUG
    //PrintGrammar();

    // Parsing
    InitCYKTable(token_count);
    for (int i = 1; i < cyk_table_dim; i ++) {
        for (int col = 0; col < (cyk_table_dim - i); col ++) {
            ParseCell(i, col);
        }
    }

    // Print CYK table
    printf("\nCYK Table:\n");
    PrintCYKTable();
    printf("\nStart Variable: %d\n", start_variable_id);

    // Check validity
    printf("\n");
    if (VarIsInCell(cyk_table_dim - 1, 0, start_variable_id)) {
        printf("ACCEPTED\n");
    } else {
        printf("REJECTED\n");
    }

    DeallocCYKTable();
    CloseCNFFile();

}

int GetVariableId(char* var_name) {

    for (int i = 0; i < variable_count; i ++) {
        if (strcmp(variables[i], var_name) == 0) {
            return i;
        }
    }
    return -1;

}

int GetTerminalId(char* term_name) {

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

void PrintGrammar() {

    printf("\n");
    printf("VARS %d\n", variable_count);
    for (int i = 0; i < variable_count; i ++) {
        printf("%s\n", variables[i]);
    }

    printf("\n");
    printf("START VAR\n");
    printf("%d\n", start_variable_id);

    printf("\n");
    printf("TERM PROD %d\n", term_prod_count);
    for (int i = 0; i < term_prod_count; i ++) {
        printf("%d %d\n", term_prods[i].source_id, term_prods[i].term_id);
    }

    printf("\n");
    printf("VAR PROD %d\n", var_prod_count);
    for (int i = 0; i < var_prod_count; i ++) {
        printf("%d %d %d\n", var_prods[i].source_id, var_prods[i].prod1_id, var_prods[i].prod2_id);
    }

}
