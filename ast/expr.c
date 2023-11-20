#include "expr.h"

#include "utils/mallocs.h"


struct ast_expr * ast_expr_new_binary(
        struct position position,
        enum ast_expr_binary_op op,
        struct ast_expr * lhs,
        struct ast_expr * rhs
) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_BINARY;
    result->binary = (struct ast_expr_binary) {
        .op = op,
        .lhs = lhs,
        .rhs = rhs,
    };

    return result;
}

struct ast_expr * ast_expr_new_unary(struct position position, enum ast_expr_unary_op op, struct ast_expr * expr) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_UNARY;
    result->unary = (struct ast_expr_unary) {
        .op = op,
        .expr = expr,
    };

    return result;
}

struct ast_expr * ast_expr_new_braces(struct position position, struct ast_expr * expr) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_BRACES;
    result->braces = (struct ast_expr_braces) {
        .expr = expr,
    };

    return result;
}

struct ast_expr * ast_expr_new_call(struct position position, struct ast_expr * function, struct ast_expr_list arguments) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_CALL;
    result->call = (struct ast_expr_call) {
        .function = function,
        .arguments = arguments,
    };

    return result;
}

struct ast_expr * ast_expr_new_indexer(struct position position, struct ast_expr * value, struct ast_expr_list indices) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_INDEXER;
    result->indexer = (struct ast_expr_indexer) {
        .value = value,
        .indices = indices,
    };

    return result;
}

struct ast_expr * ast_expr_new_place(struct position position, char * id) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_PLACE;
    result->place = (struct ast_expr_place) {
        .id = id,
    };

    return result;
}

struct ast_expr * ast_expr_new_literal(struct position position, struct ast_literal * literal) {
    struct ast_expr * result = mallocs(sizeof(struct ast_expr));

    result->position = position;

    result->_type = AST_EXPR_TYPE_LITERAL;
    result->literal = (struct ast_expr_literal) {
        .value = literal,
    };

    return result;
}

void ast_expr_delete(struct ast_expr * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case AST_EXPR_TYPE_BINARY:
            ast_expr_delete(value->binary.lhs);
            ast_expr_delete(value->binary.rhs);
            break;

        case AST_EXPR_TYPE_UNARY:
            ast_expr_delete(value->unary.expr);
            break;

        case AST_EXPR_TYPE_BRACES:
            ast_expr_delete(value->braces.expr);
            break;

        case AST_EXPR_TYPE_CALL:
            ast_expr_delete(value->call.function);
            ast_expr_list_fini(&value->call.arguments);
            break;

        case AST_EXPR_TYPE_INDEXER:
            ast_expr_delete(value->indexer.value);
            ast_expr_list_fini(&value->indexer.indices);
            break;

        case AST_EXPR_TYPE_PLACE:
            free(value->place.id);
            break;

        case AST_EXPR_TYPE_LITERAL:
            ast_literal_delete(value->literal.value);
            break;
    }

    free(value);
}

struct ast_expr_list ast_expr_list_init(void) {
    return (struct ast_expr_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_expr *)),
    };
}

void ast_expr_list_append(struct ast_expr_list * list, struct ast_expr * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_expr ** const new_values = reallocs(list->values, sizeof(struct ast_expr *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_expr_list_fini(struct ast_expr_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_expr_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct ast_expr_list) { 0 };
}
