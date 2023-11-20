#include "expr.h"

#include "utils/mallocs.h"


struct flow_graph_expr * flow_graph_expr_new_binary(
        struct position position,
        enum flow_graph_expr_binary_op op,
        struct flow_graph_expr * lhs,
        struct flow_graph_expr * rhs
) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_BINARY;
    result->binary = (struct flow_graph_expr_binary) {
        .op = op,
        .lhs = lhs,
        .rhs = rhs,
    };

    return result;
}

struct flow_graph_expr * flow_graph_expr_new_unary(
        struct position position,
        enum flow_graph_expr_unary_op op,
        struct flow_graph_expr * value
) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_UNARY;
    result->unary = (struct flow_graph_expr_unary) {
            .op = op,
            .value = value,
    };

    return result;
}

struct flow_graph_expr * flow_graph_expr_new_call(struct position position, struct flow_graph_subroutine * subroutine) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_CALL;
    result->call = (struct flow_graph_expr_call) {
            .subroutine = subroutine,
            .args = flow_graph_expr_list_init(),
    };

    return result;
}

struct flow_graph_expr * flow_graph_expr_new_indexer(struct position position, struct flow_graph_expr * value) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_INDEXER;
    result->indexer = (struct flow_graph_expr_indexer) {
            .value = value,
            .indices = flow_graph_expr_list_init(),
    };

    return result;
}

struct flow_graph_expr * flow_graph_expr_new_local(struct position position, struct flow_graph_local * local) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_LOCAL;
    result->local = (struct flow_graph_expr_local) {
            .local = local,
    };

    return result;
}

struct flow_graph_expr * flow_graph_expr_new_literal(struct position position, struct flow_graph_literal * literal) {
    struct flow_graph_expr * const result = mallocs(sizeof(struct flow_graph_expr));

    result->position = position;
    result->type = NULL;

    result->_type = FLOW_GRAPH_EXPR_TYPE_LITERAL;
    result->literal = (struct flow_graph_expr_literal) {
            .literal = literal,
    };

    return result;
}

void flow_graph_expr_delete(struct flow_graph_expr * expr) {
    if (!expr) {
        return;
    }

    ast_type_reference_delete(expr->type);

    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_BINARY:
            flow_graph_expr_delete(expr->binary.lhs);
            flow_graph_expr_delete(expr->binary.rhs);
            break;

        case FLOW_GRAPH_EXPR_TYPE_UNARY:
            flow_graph_expr_delete(expr->unary.value);
            break;

        case FLOW_GRAPH_EXPR_TYPE_CALL:
            flow_graph_expr_list_fini(&expr->call.args);
            break;

        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
            flow_graph_expr_delete(expr->indexer.value);
            flow_graph_expr_list_fini(&expr->indexer.indices);
            break;

        case FLOW_GRAPH_EXPR_TYPE_LOCAL:
            break;

        case FLOW_GRAPH_EXPR_TYPE_LITERAL:
            flow_graph_literal_delete(expr->literal.literal);
            break;
    }

    free(expr);
}

struct flow_graph_expr_list flow_graph_expr_list_init(void) {
    return (struct flow_graph_expr_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct flow_graph_expr *)),
    };
}

void flow_graph_expr_list_append(struct flow_graph_expr_list * list, struct flow_graph_expr * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct flow_graph_expr ** const new_values =
                reallocs(list->values, sizeof(struct flow_graph_expr *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void flow_graph_expr_list_fini(struct flow_graph_expr_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        flow_graph_expr_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct flow_graph_expr_list) { 0 };
}
