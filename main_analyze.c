#include <stdbool.h>
#include <stdio.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "flow_graph.h"
#include "ast_analyze/source.h"
#include "ast_analyze/error.h"
#include "ast_analyze/analyze.h"
#include "codegen/generate.h"
#include "flow_graph_display.h"
#include "utils/mallocs.h"


static const char ** input_filenames;
static size_t input_filenames_count;
static const char * output_filename;
static bool graphs = false;

static bool parse_args(int argc, char * argv[]) {
    if (argc < 3) {
        fputs("Invalid number of arguments.\n", stderr);
        return false;
    }

    int offset = 1;
    if (strcmp(argv[1], "-a") == 0) {
        graphs = true;
        ++offset;
    }

    input_filenames_count = argc - offset - 1;
    input_filenames = mallocs(sizeof(char *) * input_filenames_count);

    for (size_t i = 0; i < input_filenames_count; ++i) {
        input_filenames[i] = argv[i + offset];
    }

    output_filename = argv[input_filenames_count + 1];
    return true;
}

static int parse_files(struct ast_analyze_source_list * sources) {
    int result = 0;

    for (size_t i = 0; i < input_filenames_count; ++i) {
        FILE * const input_file = fopen(input_filenames[i], "r");

        if (!input_file) {
            perror("Bad input file");
            result = 2;
            goto end;
        }

        const YY_BUFFER_STATE buffer_state = yy_create_buffer(input_file, 4096);
        yy_switch_to_buffer(buffer_state);

        struct ast_source * ast = NULL;
        char * error = NULL;

        switch (yyparse(&ast, &error)) {
            case 1:
                fprintf(stderr, "Parsing failed: %s.\n", error);
                result = 3;
                goto fail;

            case 2:
                fputs("Memory exhausted.\n", stderr);
                result = -1;
                goto fail;
        }

        ast_analyze_source_list_append(sources, ast_analyze_source_init(input_filenames[i], ast));

fail:
        free(error);
        yy_delete_buffer(buffer_state);
        fclose(input_file);

        if (result) {
            break;
        }
    }

end:
    return result;
}

static void print_depgraph_expr(const struct flow_graph_expr * expr, FILE * output) {
    if (!expr) {
        return;
    }

    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_BINARY:
            print_depgraph_expr(expr->binary.lhs, output);
            print_depgraph_expr(expr->binary.rhs, output);
            break;

        case FLOW_GRAPH_EXPR_TYPE_UNARY:
            print_depgraph_expr(expr->unary.value, output);
            break;

        case FLOW_GRAPH_EXPR_TYPE_CALL:
            fprintf(output, "- %s from file %s\n", expr->call.subroutine->id, expr->call.subroutine->filename);

            for (size_t i = 0; i < expr->call.args.size; ++i) {
                print_depgraph_expr(expr->call.args.values[i], output);
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
            print_depgraph_expr(expr->indexer.value, output);

            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                print_depgraph_expr(expr->indexer.indices.values[i], output);
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_LOCAL:
        case FLOW_GRAPH_EXPR_TYPE_LITERAL:
            break;
    }
}

static void print_depgraph(const struct flow_graph_subroutine_list * subroutines) {
    const char * path = "";

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * subroutine = subroutines->values[i];

        if (strcmp(subroutine->id, "main") == 0) {
            path = subroutine->filename;
            break;
        }
    }

    char * dir_path = strdup(path);

    for (size_t i = strlen(dir_path); i > 0; --i) {
        if (i == 1 || dir_path[i - 1] == '/') {
            dir_path[i - 1] = '\0';
            break;
        }
    }

    char output_path[1024];
    snprintf(output_path, 1024, "%s/depgraph.txt", dir_path);
    free(dir_path);

    FILE * const file = fopen(output_path, "w");
    if (!file) {
        return;
    }

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        if (!subroutine->defined) {
            continue;
        }

        fprintf(file, "Subroutine %s calls:\n", subroutine->id);

        for (size_t j = 0; j < subroutine->nodes.size; ++j) {
            const struct flow_graph_node * const node = subroutine->nodes.values[j];

            switch (node->_type) {
                case FLOW_GRAPH_NODE_TYPE_EXPR:
                    print_depgraph_expr(node->expr.expr, file);
                    break;

                case FLOW_GRAPH_NODE_TYPE_COND:
                    print_depgraph_expr(node->cond.cond, file);
                    break;
            }
        }
    }

    fclose(file);
}

int main(int argc, char * argv[]) {
    int result = 0;

    if (!parse_args(argc, argv)) {
        fprintf(stderr, "Usage: %s -a <input filename...> <output directory path>\n", argv[0]);
        fprintf(stderr, "       %s <input filename...> <output filename>\n", argv[0]);
        return 1;
    }

    struct ast_analyze_source_list sources = ast_analyze_source_list_init();

    result = parse_files(&sources);
    if (result) {
        goto end_sources;
    }

    struct flow_graph_subroutine_list subroutines;
    struct ast_analyze_error_list errors;

    ast_analyze(&sources, &subroutines, &errors);

    if (errors.size > 0) {
        printf("Errors:\n");

        for (size_t i = 0; i < errors.size; ++i) {
            const struct ast_analyze_error * const err = &errors.values[i];

            printf("- %s in file \"%s\" at %zu:%zu\n",
                   err->message, err->filename, err->position.row, err->position.column);
        }

        result = 4;
    }

    if (graphs) {
        for (size_t i = 0; i < subroutines.size; ++i) {
            const struct flow_graph_subroutine *const subroutine = subroutines.values[i];

            char path[1024];
            snprintf(path, 1024, "%s/%s.%s.txt", output_filename, subroutine->filename, subroutine->id);
            FILE *const output_file = fopen(path, "w");
            if (!output_file) {
                perror("Bad output file");
                result = 2;
                goto end_graph;
            }

            flow_graph_display(subroutine, output_file);

            fclose(output_file);
        }

        print_depgraph(&subroutines);
    } else if (errors.size == 0) {
        struct codegen_asm_list code = codegen_generate(subroutines);

        FILE *const output_file = fopen(output_filename, "w");
        if (!output_file) {
            perror("Bad output file");
            result = 2;
            goto end_graph;
        }

        fputs(codegen_header, output_file);
        codegen_asm_list_print(code, output_file);
        fputs(codegen_builtins, output_file);
        fputs(codegen_footer, output_file);

        fclose(output_file);

        codegen_asm_list_fini(&code);
    };


end_graph:
    ast_analyze_error_list_fini(&errors);
    flow_graph_subroutine_list_fini(&subroutines);

end_sources:
    for (size_t i = 0; i < sources.size; ++i) {
        ast_source_delete(sources.values[i].source);
    }

    ast_analyze_source_list_fini(&sources);

    return result;
}
