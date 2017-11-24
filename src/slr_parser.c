#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "tokenizer.h"
#include "slr_parser.h"

/* Global variables */
Production prod[MAX_TERM_PROD_COUNT + MAX_VAR_PROD_COUNT];
int prod_count;

// SLR
Action **action_table;
int **goto_table;

/* SLR */
void InitStack(Stack *stack, int max_len) {

    stack->tab = malloc(sizeof(StackEL) * max_len);
    stack->top = -1;
    stack->max_len = max_len;

}

boolean IsStackEmpty(Stack stack) {

    return (stack.top == -1);

}

void Push(Stack *stack, StackELType type, int info) {

    StackEL stack_el;
    stack_el.type = type;
    stack_el.info = info;

    if (stack->top < stack->max_len - 1) {
        stack->top ++;
        stack->tab[stack->top] = stack_el;
    } else {
        printf("Error: Stack full\n");
    }

}

void Pop(Stack *stack, StackELType *type, int *info) {

    if (!IsStackEmpty(*stack)) {
        *type = stack->tab[stack->top].type;
        *info = stack->tab[stack->top].info;
        stack->top --;
    } else {
        printf("Error: Stack empty\n");
    }

}

void ReadCNF(char *cnf_fname) {

    FILE *file = fopen(cnf_fname, "r");
    char *ret;
    char lbuff[MAX_LINE_LEN];

    // Terminals
    char token_str[MAX_TOKEN_STR_LEN];

    term_count = 0;
    ret = fgets(lbuff, MAX_TOKEN_STR_LEN - 1, file);
    while (lbuff[0] != '%') {

        sscanf(lbuff, "%s", token_str);
        term_idx[GetTokenType(token_str)] = term_count;
        term_count ++;

        ret = fgets(lbuff, MAX_TOKEN_STR_LEN - 1, file);

    }
    term_idx[term_count] = term_count; // for $
    term_count ++;
    term_idx[term_count] = term_count; // for INVALID
    term_count ++;
    slr_term_count = term_count;

    // Variables
    char var_name_str[MAX_NAME_LEN];

    var_count = 0;
    ret = fgets(lbuff, MAX_NAME_LEN - 1, file);
    while (lbuff[0] != '%') {

        sscanf(lbuff, "%s", var_name_str);
        strcpy(var_names[var_count], var_name_str);
        var_count ++;

        ret = fgets(lbuff, MAX_NAME_LEN - 1, file);

    }
    slr_var_count = var_count;

    // Productions
    char source[MAX_NAME_LEN];
    char prod1[MAX_NAME_LEN];
    char prod2[MAX_NAME_LEN];

    prod_count = 0;
    ret = fgets(lbuff, MAX_LINE_LEN - 1, file);
    while (lbuff[0] != '%') {

        Production prod;
        if (sscanf(lbuff, "%s -> %s %s", source, prod1, prod2) == 2) {
            prod.type = TERM_PROD;
            prod.source_idx = GetVarIdxByName(source);
            prod.term_idx = GetTermIdxByName(prod1);
        } else {
            prod.type = VAR_PROD;
            prod.source_idx = GetVarIdxByName(source);
            prod.var1_idx = GetVarIdxByName(prod1);
            prod.var2_idx = GetVarIdxByName(prod2);
        }
        productions[prod_count] = prod;
        prod_count ++;

        ret = fgets(lbuff, MAX_LINE_LEN - 1, file);

    }
    slr_prod_count = prod_count;

    // State count
    ret = fgets(lbuff, MAX_LINE_LEN - 1, file);
    sscanf(lbuff, "%d", &slr_state_count);

    fclose(file);

    /*/ DEBUG
    for (int i = 0; i < term_count; i ++) {
        printf("Term | info: %3d, Type: %3d\n", term_idx[i], i);
    }

    for (int i = 0; i < var_count; i ++) {
        printf("Var  | info: %3d, Name: %s\n", i, var_names[i]);
    }
    /*/

}

void ReadGotoTable(char *goto_fname) {

    FILE *file = fopen(goto_fname, "r");
    char wbuff[10];

    for (int i = 0; i < slr_state_count; i ++) {
        for (int j = 0; j < slr_var_count; j ++) {
            fscanf(file, "%s", wbuff);

            if (wbuff[0] != ';') {

                int goto_state = 0;
                for (int k = 0; k < strlen(wbuff); k ++) {
                    if (wbuff[k] == ';') {
                        wbuff[k] = '\0';
                        break;
                    } else {
                        goto_state *= 10;
                        goto_state += wbuff[k] - '0';
                    }
                }
                goto_table[i][j] = goto_state;

            } else {
                goto_table[i][j] = -1;
            }

        }
    }

    fclose(file);

}

void ReadActionTable(char *action_fname) {

    FILE *file = fopen(action_fname, "r");
    char wbuff[10];

    for (int i = 0; i < slr_state_count; i ++) {

        for (int j = 0; j < slr_term_count - 1; j ++) {
            fscanf(file, "%s", wbuff);

            Action action;

            if (wbuff[0] == ';') {

                action.type = REJECT;
                action.info = -1;
                action_table[i][j] = action;

            } else if (wbuff[0] == 'a') {

                action.type = ACCEPT;
                action.info = -1;
                action_table[i][j] = action;

            } else {

                int info = 0;

                if (wbuff[0] == 'r') {

                    action.type = REDUCE;
                    for (int k = 1; k < strlen(wbuff); k ++) {
                        if (wbuff[k] == ';' || wbuff[k] == '/') {
                            wbuff[k] = '\0';
                            break;
                        } else {
                            info *= 10;
                            info += wbuff[k] - '0';
                        }
                    }

                } else {

                    action.type = SHIFT;
                    int start = 1;;

                    // If conflicts with reduce, choose reduce
                    for (int k = 1; k < strlen(wbuff); k ++) {
                        if (wbuff[k] == '/') {
                            if (wbuff[k + 1] == 'r') {
                                action.type = REDUCE;
                                start = k + 2;
                            } else {
                                start = 1;
                            }
                            break;
                        } else if (wbuff[k] == ';') {
                            start = 1;
                            break;
                        }
                    }

                    for (int k = start; k < strlen(wbuff); k ++) {
                        if (wbuff[k] == ';' || wbuff[k] == '/') {
                            wbuff[k] = '\0';
                            break;
                        } else {
                            info *= 10;
                            info += wbuff[k] - '0';
                        }
                    }

                }

                action.info = info;
                action_table[i][j] = action;

            }

        }

        // Add INVALID
        Action action;
        action.type = REJECT;
        action.info = -1;
        action_table[i][slr_term_count - 1] = action;

    }

    fclose(file);

}

int GetVarIdxByName(char *var_name) {

    for (int i = 0; i < var_count; i ++) {
        if (strcmp(var_names[i], var_name) == 0) {
            return i;
        }
    }
    printf("Error: var not found.\n");
    return -1;

}

int GetTermIdxByName(char *term_name) {

    int token_type = GetTokenType(term_name);
    int info = term_idx[token_type];
    if (info == -1) {
        printf("Error: term not found.\n");
    }
    return info;

}

int GetTermIdxByType(int term_type) {

    return term_idx[term_type];

}

void InitSLRTable() {

    // Action table
    action_table = malloc(sizeof(Action *) * slr_state_count);
    for (int i = 0; i < slr_state_count; i ++) {
        action_table[i] = malloc(sizeof(Action) * (slr_term_count + 1));
    }

    // Goto table
    goto_table = malloc(sizeof(int *) * slr_state_count);
    for (int i = 0; i < slr_state_count; i ++) {
        goto_table[i] = malloc(sizeof(int) * slr_var_count);
    }

}

void DeallocSLRTable(){

    // Action table
    for (int i = 0; i < slr_state_count; i ++) {
        free(action_table[i]);
    }
    free(action_table);

    // Goto table
    for (int i = 0; i < slr_state_count; i ++) {
        free(goto_table[i]);
    }
    free(goto_table);

}

void SLR(char *source_fname, char *cnf_fmatted_fname, char *goto_fname, char *action_fname) {

    // Init cnf
    ReadCNF(cnf_fmatted_fname);

    // Init table
    InitSLRTable();
    ReadGotoTable(goto_fname);
    ReadActionTable(action_fname);

    // Init stack
    Stack stack;
    InitStack(&stack, MAX_STACK_LEN);

    // TODO: INVALID gaada di tabel, jadi harus di deal manual

    // Add '$' to end of tokens
    tokens[token_count].type = TERMINATE_TOKEN;
    strcpy(tokens[token_count].str, "$");
    tokens[token_count].line = 0;
    tokens[token_count].col = 0;
    token_count ++;

    // Parsing
    int term_pointer = 0;
    int status = 0;
    Token err_token;

    Push(&stack, STATE, 0);

    while(status == 0) {

        if (stack.tab[stack.top].type == STATE) {

            int curr_state = stack.tab[stack.top].info;
            int curr_term_idx = GetTermIdxByType(tokens[term_pointer].type);
            Action action = action_table[curr_state][curr_term_idx];

            // DEBUG
            // printf("stack: %d\n", stack.tab[stack.top].info);
            // printf("term: %d\n", tokens[term_pointer].type);
            // char action_type;
            // if (action.type == SHIFT) action_type = 's';
            // else if (action.type == REDUCE) action_type = 'r';
            // else if (action.type == ACCEPT) action_type = 'A';
            // else  action_type = 'R';
            // printf("%c%d\n\n", action_type, action.info);

            if (action.type == SHIFT) {

                Push(&stack, TERMINAL, curr_term_idx);
                Push(&stack, STATE, action.info);
                term_pointer ++;

            } else if (action.type == REDUCE){

                Production prod = productions[action.info];
                if (prod.type == VAR_PROD) {
                    StackELType el_type;
                    int el_info;
                    Pop(&stack, &el_type, &el_info);
                    Pop(&stack, &el_type, &el_info);
                    Pop(&stack, &el_type, &el_info);
                    Pop(&stack, &el_type, &el_info);
                    Push(&stack, VARIABLE, prod.source_idx);
                } else {
                    StackELType el_type;
                    int el_info;
                    Pop(&stack, &el_type, &el_info);
                    Pop(&stack, &el_type, &el_info);
                    Push(&stack, VARIABLE, prod.source_idx);
                }

            } else if (action.type == ACCEPT) {
                // Accepted
                status  = 1;
            } else {
                // Rejected
                status = -1;
                err_token = tokens[term_pointer];
            }

        } else { // var or term

            int curr_var_idx = stack.tab[stack.top].info;
            int prev_state = stack.tab[stack.top - 1].info;
            int new_state = goto_table[prev_state][curr_var_idx];

            // DEBUG
            // printf("%d\n", new_state);

            if (new_state == -1) {
                // Rejected
                status = -1;
                err_token = tokens[term_pointer];
            } else {
                Push(&stack, STATE, new_state);
            }

        }

    }

    if (status == 1) {
        printf("ACCEPTED\n");
    } else {
        printf("REJECTED\n");
        PrintError(source_fname, err_token);
    }

    DeallocSLRTable();

}

void PrintError(char *source_fname, Token token) {

    FILE *file = fopen(source_fname, "r");

    char term_name[MAX_TOKEN_STR_LEN];
    GetTokenName(term_name, token.type);
    printf("\nSyntax error at line %d, col %d (UNEXPECTED TOKEN: %s):\n\n", token.line, token.col, term_name);

    char lbuff[MAX_LINE_LEN];
    char cbuff;
    for (int i = 0; i < token.line; i ++) {
        int j;
        for (j = 0; j < MAX_LINE_LEN; j ++) {
            cbuff = fgetc(file);
            if (cbuff == '\n' || cbuff == EOF) {
                lbuff[j] = '\n';
                lbuff[j + 1] = '\0';
                break;
            } else {
                lbuff[j] = cbuff;
            }
        }
    }
    printf("%s", lbuff);

    for (int i = 0; i < token.col - 1; i ++) {
        printf(" ");
    }
    printf("%s^", "\x1B[31m");

    for (int i = 0; i < strlen(token.str) - 1; i ++) {
        printf("~");
    }
    printf("%s\n", "\x1B[0m");

}
