#include "ast_display.h"

#include <stdio.h>
#include <string.h>

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
        [AST_EXPR_BINARY_OP_ASSIGNMENT_PLUS] = "+=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_MINUS] = "-=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_MULTIPLY] = "*=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_DIVIDE] = "/=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_REMAINDER] = "%=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_AND] = "&=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_OR] = "|=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_XOR] = "^=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_AND] = "&&=",
        [AST_EXPR_BINARY_OP_ASSIGNMENT_OR] = "||=",
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

void ast_display_source(struct ast_source * source, const char * label, size_t indent, FILE * output) {
    if (!source) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    print_indent(label, indent, output);
    fprintf(output, "<source>");
    print_position_ln(source->position, output);

    for (size_t i = 0; i < source->items.size; ++i) {
        ast_display_source_item(source->items.values[i], index_label("items", i), indent + 2, output);
    }
}

void ast_display_source_item(struct ast_source_item * source_item, const char * label, size_t indent, FILE * output) {
    if (!source_item) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (source_item->_type) {
        case AST_SOURCE_ITEM_TYPE_FUNC_DECL:
            print_indent(label, indent, output);
            fprintf(output, "<source_item:func_decl>");
            print_position_ln(source_item->position, output);
            ast_display_function_signature(source_item->func_decl.signature, "signature", indent + 2, output);
            ast_display_stmt(source_item->func_decl.body, "body", indent + 2, output);
            break;
    }
}

void ast_display_function_signature(
        struct ast_function_signature * function_signature,
        const char * label,
        size_t indent, FILE * output
) {
    if (!function_signature) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    print_indent(label, indent, output);
    fprintf(output, "<function_signature> \"%s\"", function_signature->id);
    print_position_ln(function_signature->position, output);

    ast_display_type_reference(function_signature->return_type, "return_type", indent + 2, output);

    for (size_t i = 0; i < function_signature->args.size; ++i) {
        print_indent(index_label("args", i), indent + 2, output);
        fprintf(output, "<function_signature_arg> \"%s\"", function_signature->args.values[i].id);
        print_position_ln(function_signature->args.values[i].position, output);

        ast_display_type_reference(function_signature->args.values[i].type, "type", indent + 4, output);
    }
}

void ast_display_type_reference(struct ast_type_reference * type_reference, const char * label, size_t indent, FILE * output) {
    if (!type_reference) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (type_reference->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            print_indent(label, indent, output);
            fprintf(output, "<type_reference:builtin> %s", TYPE_REFERENCE_BUILTIN_TYPES[type_reference->builtin.type]);
            print_position_ln(type_reference->position, output);
            break;

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
            print_indent(label, indent, output);
            fprintf(output, "<type_reference:custom> \"%s\"", type_reference->custom.id);
            print_position_ln(type_reference->position, output);
            break;

        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            print_indent(label, indent, output);
            fprintf(output, "<type_reference:array> %zu", type_reference->array.axes);
            print_position_ln(type_reference->position, output);

            ast_display_type_reference(type_reference->array.type, "type", indent + 2, output);
            break;
    }
}

void ast_display_stmt(struct ast_stmt * stmt, const char * label, size_t indent, FILE * output) {
    if (!stmt) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (stmt->_type) {
        case AST_STMT_TYPE_VAR:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:var>");
            print_position_ln(stmt->position, output);

            ast_display_type_reference(stmt->var.type, "type", indent + 2, output);
            for (size_t i = 0; i < stmt->var.ids.size; ++i) {
                print_indent(index_label("ids", i), indent + 2, output);
                fprintf(output, "<stmt_var_id> \"%s\"", stmt->var.ids.values[i].id);
                print_position_ln(stmt->var.ids.values[i].position, output);

                ast_display_expr(stmt->var.ids.values[i].value, "value", indent + 4, output);
            }

            break;

        case AST_STMT_TYPE_IF:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:if>");
            print_position_ln(stmt->position, output);

            ast_display_expr(stmt->_if.condition, "condition", indent + 2, output);
            ast_display_stmt(stmt->_if.then_branch, "then_branch", indent + 2, output);
            ast_display_stmt(stmt->_if.else_branch, "else_branch", indent + 2, output);
            break;

        case AST_STMT_TYPE_BLOCK:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:block>");
            print_position_ln(stmt->position, output);

            for (size_t i = 0; i < stmt->block.stmts.size; ++i) {
                ast_display_stmt(stmt->block.stmts.values[i], index_label("stmts", i), indent + 2, output);
            }

            break;

        case AST_STMT_TYPE_WHILE:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:while>");
            print_position_ln(stmt->position, output);

            ast_display_expr(stmt->_while.condition, "condition", indent + 2, output);
            ast_display_stmt(stmt->_while.body, "body", indent + 2, output);
            break;

        case AST_STMT_TYPE_DO:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:do>");
            print_position_ln(stmt->position, output);

            ast_display_stmt(stmt->_do.body, "body", indent + 2, output);
            ast_display_expr(stmt->_do.condition, "condition", indent + 2, output);
            break;

        case AST_STMT_TYPE_BREAK:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:break>");
            print_position_ln(stmt->position, output);
            break;

        case AST_STMT_TYPE_EXPR:
            print_indent(label, indent, output);
            fprintf(output, "<stmt:expr>");
            print_position_ln(stmt->position, output);

            ast_display_expr(stmt->expr.expr, "expr", indent + 2, output);
            break;
    }
}

void ast_display_expr(struct ast_expr * expr, const char * label, size_t indent, FILE * output) {
    if (!expr) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (expr->_type) {
        case AST_EXPR_TYPE_BINARY:
            print_indent(label, indent, output);
            fprintf(output, "<expr:binary> %s", EXPR_BINARY_OPS[expr->binary.op]);
            print_position_ln(expr->position, output);

            ast_display_expr(expr->binary.lhs, "lhs", indent + 2, output);
            ast_display_expr(expr->binary.rhs, "rhs", indent + 2, output);
            break;

        case AST_EXPR_TYPE_UNARY:
            print_indent(label, indent, output);
            fprintf(output, "<expr:unary> %s", EXPR_UNARY_OPS[expr->unary.op]);
            print_position_ln(expr->position, output);

            ast_display_expr(expr->unary.expr, "expr", indent + 2, output);
            break;

        case AST_EXPR_TYPE_BRACES:
            print_indent(label, indent, output);
            fprintf(output, "<expr:braces>");
            print_position_ln(expr->position, output);

            ast_display_expr(expr->braces.expr, "expr", indent + 2, output);
            break;

        case AST_EXPR_TYPE_CALL:
            print_indent(label, indent, output);
            fprintf(output, "<expr:call>");
            print_position_ln(expr->position, output);

            ast_display_expr(expr->call.function, "function", indent + 2, output);
            for (size_t i = 0; i < expr->call.arguments.size; ++i) {
                ast_display_expr(expr->call.arguments.values[i], index_label("arguments", i), indent + 2, output);
            }

            break;

        case AST_EXPR_TYPE_INDEXER:
            print_indent(label, indent, output);
            fprintf(output, "<expr:indexer>");
            print_position_ln(expr->position, output);

            ast_display_expr(expr->indexer.value, "value", indent + 2, output);
            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                ast_display_expr(expr->indexer.indices.values[i], index_label("indices", i), indent + 2, output);
            }

            break;

        case AST_EXPR_TYPE_PLACE:
            print_indent(label, indent, output);
            fprintf(output, "<expr:place> \"%s\"", expr->place.id);
            print_position_ln(expr->position, output);
            break;

        case AST_EXPR_TYPE_LITERAL:
            print_indent(label, indent, output);
            fprintf(output, "<expr:literal>");
            print_position_ln(expr->position, output);

            ast_display_literal(expr->literal.value, "value", indent + 2, output);
            break;
    }
}

void ast_display_literal(struct ast_literal * literal, const char * label, size_t indent, FILE * output) {
    if (!literal) {
        print_indent(label, indent, output);
        fprintf(output, "NULL\n");
        return;
    }

    switch (literal->_type) {
        case AST_LITERAL_TYPE_BOOL:
            print_indent(label, indent, output);
            fprintf(output, "<literal:bool> %s", literal->_bool.value ? "true" : "false");
            print_position_ln(literal->position, output);
            break;

        case AST_LITERAL_TYPE_STR:
            print_indent(label, indent, output);
            fprintf(output, "<literal:str> %s", literal->str.value);
            print_position_ln(literal->position, output);
            break;

        case AST_LITERAL_TYPE_CHAR:
            print_indent(label, indent, output);
            fprintf(output, "<literal:char> '%c'", literal->_char.value);
            print_position_ln(literal->position, output);
            break;

        case AST_LITERAL_TYPE_HEX:
            print_indent(label, indent, output);
            fprintf(output, "<literal:hex> \"%s\"", literal->hex.value);
            print_position_ln(literal->position, output);
            break;

        case AST_LITERAL_TYPE_BITS:
            print_indent(label, indent, output);
            fprintf(output, "<literal:bits> \"%s\"", literal->bits.value);
            print_position_ln(literal->position, output);
            break;

        case AST_LITERAL_TYPE_DEC:
            print_indent(label, indent, output);
            fprintf(output, "<literal:dec> \"%s\"", literal->dec.value);
            print_position_ln(literal->position, output);
            break;
    }
}
