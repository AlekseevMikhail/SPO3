#include <stdbool.h>
#include <stdio.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "ast_display.h"


static const char * input_filename;
static const char * output_filename;

static bool parse_args(int argc, char * argv[]) {
    if (argc != 3) {
        fputs("Invalid number of arguments.\n", stderr);
        return false;
    }

    input_filename = argv[1];
    output_filename = argv[2];
    return true;
}

int main(int argc, char * argv[]) {
    int result = 0;

    if (!parse_args(argc, argv)) {
        fprintf(stderr, "Usage: %s <input filename> <output filename>\n", argv[0]);
        return 1;
    }

    FILE * const input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Bad input file");
        result = 2;
        goto end;
    }

    FILE * const output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Bad output file");
        result = 2;
        goto end_input_file;
    }

    const YY_BUFFER_STATE buffer_state = yy_create_buffer(input_file, 4096);
    yy_switch_to_buffer(buffer_state);

    struct ast_source * ast = NULL;
    char * error = NULL;

    switch (yyparse(&ast, &error)) {
        case 1:
            fprintf(stderr, "Parsing failed: %s.\n", error);
            result = 3;
            goto end_error;

        case 2:
            fputs("Memory exhausted.\n", stderr);
            result = -1;
            goto end_error;
    }

    ast_display_source(ast, "AST", 0, output_file);

// end_ast:
    ast_source_delete(ast);

end_error:
    free(error);

// end_output_file:
    fclose(output_file);

end_input_file:
    fclose(input_file);

end:
    return result;
}
