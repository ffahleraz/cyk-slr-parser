#ifndef SLR_PARSER_H_
#define SLR_PARSER_H_

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

// SLR
typedef enum ProductionType {
    VAR_PROD, TERM_PROD,
} ProductionType;

typedef struct Production {
    ProductionType type;
    int source_idx;
    int term_idx;
    int var1_idx;
    int var2_idx;
} Production;

typedef enum ActionType {
    SHIFT, REDUCE, ACCEPT, REJECT
} ActionType;

typedef enum StackELType {
    STATE, TERMINAL, VARIABLE
} StackELType;

typedef struct Action {
    ActionType type;
    int info; // state or prod idx
} Action;

typedef struct StackEL {
    StackELType type;
    int info;
} StackEL;

typedef struct Stack {
    StackEL *tab;
    int top;
    int max_len;
} Stack;

/* Global variables */
// Database for index
// Index: token type, Value: index
TokenType term_idx[TERM_COUNT]; // minus INVALID
int term_count; // includes '$', last idx is '$'

// Index: index, Value: var name
char var_names[MAX_VAR_COUNT][MAX_NAME_LEN];
int var_count;

Production productions[MAX_TERM_PROD_COUNT + MAX_VAR_PROD_COUNT];
int prod_count;

// SLR
Action **action_table;
int **goto_table;
int slr_state_count;
int slr_prod_count;
int slr_var_count;
int slr_term_count; // includes '$'

/* Prototypes */
/* SLR */
void InitStack(Stack *stack, int max_len);
boolean IsStackEmpty(Stack stack);
void Push(Stack *stack, StackELType type, int info);
void Pop(Stack *stack, StackELType *type, int *info);

void ReadCNF(char *cnf_fname);
void ReadGotoTable(char *goto_fname);
void ReadActionTable(char *action_fname);

int GetVarIdxByName(char *var_name);
int GetTermIdxByName(char *term_name);
int GetTermIdxByType(int term_type);

void InitSLRTable();
void DeallocSLRTable();

void SLR(char *source_fname, char *cnf_fmatted_fname, char *goto_fname, char *action_fname);

void PrintError(char *source_fname, Token token);

#endif
