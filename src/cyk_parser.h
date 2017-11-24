#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "tokenizer.h"

#define MAX_TERM_PROD_COUNT 200
#define MAX_VAR_PROD_COUNT 200
#define MAX_VAR_COUNT 200
#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200
#define MAX_STACK_LEN 200

// CYK
typedef struct TermProduction {
    int source_id;
    int term_id;
} TermProduction;

typedef struct VarProduction {
    int source_id;
    int prod1_id;
    int prod2_id;
} VarProduction;

typedef struct CYKCell {
    int *vars;
    int var_count;
} CYKCell;

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

/* Prototypes */
/* CYK */
void OpenCNFFile(char *file_name);
void CloseCNFFile();

void AdvanceLine();
void IgnoreBlankLine();
boolean IsAtBlankLine();
boolean IsAtEndLine();

void ReadVariables();
void ReadStartVariable();
void ReadTerminalProductions();
void ReadVariableProductions();

boolean VarIsInCell(int row, int col, int var_id);
void AddReducedTermTo(int row, int col, int term_id);
void AddReducedVarsTo(int row, int col, int var1_id, int var2_id);
void ParseCell(int row, int col);

void InitCYKTable(int dim);
void DeallocCYKTable();
void PrintCYKTable();

void CYK(char *file_name);

// Data
int GetVariableId(char* var_name);
int GetTerminalId(char* term_name);

void PrintGrammar();

#endif
