#include <stdio.h>
#include <string.h>
#include "tokenizer.h"
#include "cyk_parser.h"
#include "slr_parser.h"

#define CNF_FNAME "cnf/cnf.txt"
#define CNF_FMATTED_FNAME "cnf/cnf_formatted.txt"
#define GOTO_TABLE_FNAME "cnf/goto_table.csv"
#define ACTION_TABLE_FNAME "cnf/action_table.csv"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: ./parser <source_file_name>\n");
        return 1;
    }

    /* Tokenizing */
    Tokenize(argv[1]);

    // Print tokens
    // for (int i = 0; i < token_count; i ++) {
    //     Token token = tokens[i];
    //     char term_name[MAX_TOKEN_STR_LEN];
    //     GetTokenName(term_name, token.type);
    //     printf("Type: %18s, Line: %3d, Col: %3d, Lex: %s\n", term_name, token.line, token.col, token.str);
    // }

    /* CYK */
    CYK(CNF_FNAME);

    // DEBUG
    //PrintGrammar();

    /* SLR */
    printf("\nLR(1) Parsing:\n\n");
    SLR(argv[1], CNF_FMATTED_FNAME, GOTO_TABLE_FNAME, ACTION_TABLE_FNAME);
    printf("\n");

    return 0;

}
