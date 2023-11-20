#pragma once

#include <stddef.h>

#include "expr.h"
#include "utils/position.h"


enum flow_graph_node_type {

    FLOW_GRAPH_NODE_TYPE_EXPR = 0,
    FLOW_GRAPH_NODE_TYPE_COND,
};

struct flow_graph_node_expr {

    struct flow_graph_expr * expr;
    struct flow_graph_node * next;
};

struct flow_graph_node_cond {

    struct flow_graph_expr * cond;
    struct flow_graph_node * then_next;
    struct flow_graph_node * else_next;
};

struct flow_graph_node {

    size_t index;
    struct position position;
    enum flow_graph_node_type _type;

    union {
        struct flow_graph_node_expr expr;
        struct flow_graph_node_cond cond;
    };
};

struct flow_graph_node_list {

    size_t size;
    size_t capacity;
    struct flow_graph_node ** values;
};

struct flow_graph_node * flow_graph_node_new_expr(struct position position, struct flow_graph_expr * expr);
struct flow_graph_node * flow_graph_node_new_cond(struct position position, struct flow_graph_expr * cond);
void flow_graph_node_delete(struct flow_graph_node * node);

struct flow_graph_node_list flow_graph_node_list_init(void);
void flow_graph_node_list_append(struct flow_graph_node_list * list, struct flow_graph_node * value);
void flow_graph_node_list_fini(struct flow_graph_node_list * list);
