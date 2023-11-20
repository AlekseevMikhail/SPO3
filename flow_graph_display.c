#include "flow_graph_display.h"

#include <string.h>

#include "ast_display.h"
#include "utils/mallocs.h"


static const char * const EXPR_BINARY_OPS[] = {
        [AST_EXPR_BINARY_OP_ASSIGNMENT] = "=",
        [AST_EXPR_BINARY_OP_PLUS] = "+",
        [AST_EXPR_BINARY_OP_MINUS] = "-",
        [AST_EXPR_BINARY_OP_MULTIPLY] = "*",
        [AST_EXPR_BINARY_OP_DIVIDE] = "/",
        [AST_EXPR_BINARY_OP_REMAINDER] = "%",
        [AST_EXPR_BINARY_OP_BITWISE_AND] = "&",
        [AST_EXPR_BINARY_OP_BITWISE_OR] = "|",
        [AST_EXPR_BINARY_OP_BITWISE_XOR] = "^",
        [AST_EXPR_BINARY_OP_AND] = "&&",
        [AST_EXPR_BINARY_OP_OR] = "||",
        [AST_EXPR_BINARY_OP_EQ] = "==",
        [AST_EXPR_BINARY_OP_NE] = "!=",
        [AST_EXPR_BINARY_OP_LT] = "<",
        [AST_EXPR_BINARY_OP_LE] = "<=",
        [AST_EXPR_BINARY_OP_GT] = ">",
        [AST_EXPR_BINARY_OP_GE] = ">=",
        [AST_EXPR_BINARY_OP_LEFT_BITSHIFT] = "<<",
        [AST_EXPR_BINARY_OP_RIGHT_BITSHIFT] = ">>",
};

static void print_indent(const char * label, size_t indent, FILE * output) {
    for (size_t i = 0; i < indent; ++i) {
        fputc(' ', output);
    }

    fprintf(output, "- %s -> ", label);
}

static void print_position_ln(struct position position, FILE * output) {
    fprintf(output, " at %zu:%zu\n", position.row, position.column);
}

static const char * index_label(const char * label, size_t index) {
    static char * buffer = NULL;

    char * new_buffer = reallocs(buffer, strlen(label) + 23);

    buffer = new_buffer;
    sprintf(buffer, "%s[%zu]", label, index);

    return buffer;
}

static void print_str(const char * str, FILE * output) {
    const size_t len = strlen(str);

    fputc('"', output);

    for (size_t i = 0; i < len; ++i) {
        switch (str[i]) {
            case '\0':
                fputs("\\0", output);
                break;

            case '\b':
                fputs("\\b", output);
                break;

            case '\n':
                fputs("\\n", output);
                break;

            case '\r':
                fputs("\\r", output);
                break;

            case '\t':
                fputs("\\t", output);
                break;

            case '\\':
                fputs("\\\\", output);
                break;

            case '"':
                fputs("\\\"", output);
                break;

            default:
                fputc(str[i], output);
        }
    }

    fputc('"', output);
}

static void print_literal(struct flow_graph_literal * literal, const char * label, size_t indent, FILE * output) {
    if (!literal) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (literal->_type) {
        case FLOW_GRAPH_LITERAL_TYPE_BOOL:
            print_indent(label, indent, output);
            fprintf(output, "<literal:bool> %s", literal->_bool.value ? "true" : "false");
            print_position_ln(literal->position, output);
            break;

        case FLOW_GRAPH_LITERAL_TYPE_STR:
            print_indent(label, indent, output);
            fprintf(output, "<literal:str> ");
            print_str(literal->str.value, output);
            print_position_ln(literal->position, output);
            break;

        case FLOW_GRAPH_LITERAL_TYPE_CHAR:
            print_indent(label, indent, output);
            fprintf(output, "<literal:char> '%c'", literal->_char.value);
            print_position_ln(literal->position, output);
            break;

        case FLOW_GRAPH_LITERAL_TYPE_INT:
            print_indent(label, indent, output);
            fprintf(output, "<literal:int> %zu", literal->_int.value);
            print_position_ln(literal->position, output);
            break;
    }
}

static void print_type(struct ast_type_reference * type, FILE * output) {
    if (!type) {
        fprintf(output, "NULL");
        return;
    }

    switch (type->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            fprintf(output, "%s", TYPE_REFERENCE_BUILTIN_TYPES[type->builtin.type]);
            break;

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
            fprintf(output, "%s", type->custom.id);
            break;

        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            print_type(type->array.type, output);
            fputc('[', output);

            for (size_t i = 1; i < type->array.axes; ++i) {
                fputc(',', output);
            }

            fputc(']', output);
            break;
    }
}

static void print_type_ex(struct ast_type_reference * type, FILE * output) {
    print_type(type, output);

    if (type) {
        print_position_ln(type->position, output);
    } else {
        fprintf(output, "\n");
    }
}

static void print_subroutine_id(const struct flow_graph_subroutine * subroutine, FILE * output) {
    fprintf(
            output,
            "subroutine \"%s\" in file \"%s\" at %zu:%zu (%s)\n",
            subroutine->id,
            subroutine->filename,
            subroutine->position.row,
            subroutine->position.column,
            subroutine->defined ? "defined" : "not defined"
    );
}

static void print_local(const struct flow_graph_local * local, FILE * output) {
    fprintf(output, "#%zu ", local->index);
    print_type(local->type, output);
    fprintf(output, " %s", local->id);
    print_position_ln(local->position, output);
}

static void print_expr(struct flow_graph_expr * expr, const char * label, size_t indent, FILE * output) {
    if (!expr) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_BINARY:
            print_indent(label, indent, output);
            fprintf(output, "<expr:binary> %s", EXPR_BINARY_OPS[expr->binary.op]);
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_expr(expr->binary.lhs, "lhs", indent + 2, output);
            print_expr(expr->binary.rhs, "rhs", indent + 2, output);
            break;

        case FLOW_GRAPH_EXPR_TYPE_UNARY:
            print_indent(label, indent, output);
            fprintf(output, "<expr:unary> %s", EXPR_UNARY_OPS[expr->unary.op]);
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_expr(expr->unary.value, "value", indent + 2, output);
            break;

        case FLOW_GRAPH_EXPR_TYPE_CALL:
            print_indent(label, indent, output);
            fprintf(output, "<expr:call>");
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_indent("subroutine", indent + 2, output);
            print_subroutine_id(expr->call.subroutine, output);

            for (size_t i = 0; i < expr->call.args.size; ++i) {
                print_expr(expr->call.args.values[i], index_label("args", i), indent + 2, output);
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
            print_indent(label, indent, output);
            fprintf(output, "<expr:indexer>");
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_expr(expr->indexer.value, "value", indent + 2, output);
            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                print_expr(expr->indexer.indices.values[i], index_label("indices", i), indent + 2, output);
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_LOCAL:
            print_indent(label, indent, output);
            fprintf(output, "<expr:local>");
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_indent("local", indent + 2, output);
            print_local(expr->local.local, output);
            break;

        case FLOW_GRAPH_EXPR_TYPE_LITERAL:
            print_indent(label, indent, output);
            fprintf(output, "<expr:literal>");
            print_position_ln(expr->position, output);

            print_indent("type", indent + 2, output);
            print_type_ex(expr->type, output);

            print_literal(expr->literal.literal, "literal", indent + 2, output);
            break;
    }
}

static void print_node_index(struct flow_graph_node * node, FILE * output) {
    if (!node) {
        fprintf(output, "RETURN");
        return;
    }

    fprintf(output, "#%zu", node->index);
}

static void print_nodes(struct flow_graph_node * node, bool * visited, FILE * output) {
    if (!node || node->index == 0) {
        return;
    }

    if (visited[node->index - 1]) {
        return;
    }

    visited[node->index - 1] = true;

    switch (node->_type) {
        case FLOW_GRAPH_NODE_TYPE_EXPR:
            if (!node->expr.expr) {
                fprintf(output, "  - #%zu NOP", node->index);
                print_position_ln(node->position, output);
            } else {
                fprintf(output, "  - #%zu EXPR", node->index);
                print_position_ln(node->position, output);

                print_expr(node->expr.expr, "expr", 4, output);
            }

            fprintf(output, "    - next: ");
            print_node_index(node->expr.next, output);
            fprintf(output, "\n");

            print_nodes(node->expr.next, visited, output);
            break;

        case FLOW_GRAPH_NODE_TYPE_COND:
            fprintf(output, "  - #%zu COND", node->index);
            print_position_ln(node->position, output);

            print_expr(node->cond.cond, "cond", 4, output);

            fprintf(output, "    - then next: ");
            print_node_index(node->cond.then_next, output);
            fprintf(output, "\n");

            fprintf(output, "    - else next: ");
            print_node_index(node->cond.else_next, output);
            fprintf(output, "\n");

            print_nodes(node->cond.then_next, visited, output);
            print_nodes(node->cond.else_next, visited, output);
            break;
    }
}

void flow_graph_display(const struct flow_graph_subroutine * subroutine, FILE * output) {
    print_subroutine_id(subroutine, output);

    fprintf(output, "  with %zu arguments\n", subroutine->args_num);
    fprintf(output, "  returns ");
    print_type_ex(subroutine->return_type, output);

    if (subroutine->locals.size > 0) {
        fprintf(output, "- Local variables:\n");
        for (size_t i = 0; i < subroutine->locals.size; ++i) {
            fprintf(output, "  - ");
            print_local(subroutine->locals.values[i], output);
        }
    }

    if (subroutine->nodes.size > 0) {
        bool * const visited = mallocs(sizeof(bool) * subroutine->nodes.size);
        memset(visited, 0, sizeof(bool) * subroutine->nodes.size);

        fprintf(output, "- Flow graph:\n");
        print_nodes(subroutine->nodes.values[0], visited, output);

        free(visited);
    }
}
