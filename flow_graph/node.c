#include "node.h"

#include <stdlib.h>

#include "utils/mallocs.h"


struct flow_graph_node * flow_graph_node_new_expr(struct position position, struct flow_graph_expr * expr) {
    struct flow_graph_node * const result = mallocs(sizeof(struct flow_graph_node));

    result->index = 0;
    result->position = position;
    result->_type = FLOW_GRAPH_NODE_TYPE_EXPR;
    result->expr.expr = expr;
    result->expr.next = NULL;

    return result;
}

struct flow_graph_node * flow_graph_node_new_cond(struct position position, struct flow_graph_expr * cond) {
    struct flow_graph_node * const result = mallocs(sizeof(struct flow_graph_node));

    result->index = 0;
    result->position = position;
    result->_type = FLOW_GRAPH_NODE_TYPE_COND;
    result->cond.cond = cond;
    result->cond.then_next = NULL;
    result->cond.else_next = NULL;

    return result;
}

void flow_graph_node_delete(struct flow_graph_node * node) {
    if (!node) {
        return;
    }

    switch (node->_type) {
        case FLOW_GRAPH_NODE_TYPE_EXPR:
            flow_graph_expr_delete(node->expr.expr);
            break;

        case FLOW_GRAPH_NODE_TYPE_COND:
            flow_graph_expr_delete(node->cond.cond);
            break;
    }

    free(node);
}

struct flow_graph_node_list flow_graph_node_list_init(void) {
    return (struct flow_graph_node_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct flow_graph_node)),
    };
}

void flow_graph_node_list_append(struct flow_graph_node_list * list, struct flow_graph_node * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct flow_graph_node ** const new_values =
                reallocs(list->values, sizeof(struct flow_graph_node *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void flow_graph_node_list_fini(struct flow_graph_node_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        flow_graph_node_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct flow_graph_node_list) { 0 };
}
