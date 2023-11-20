#pragma once

#include <stddef.h>

#include "ast.h"
#include "flow_graph/literal.h"
#include "utils/position.h"


enum flow_graph_expr_type {

    FLOW_GRAPH_EXPR_TYPE_BINARY = 0,
    FLOW_GRAPH_EXPR_TYPE_UNARY,
    FLOW_GRAPH_EXPR_TYPE_CALL,
    FLOW_GRAPH_EXPR_TYPE_INDEXER,
    FLOW_GRAPH_EXPR_TYPE_LOCAL,
    FLOW_GRAPH_EXPR_TYPE_LITERAL,
};

enum flow_graph_expr_binary_op {

    FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT = AST_EXPR_BINARY_OP_ASSIGNMENT,
    FLOW_GRAPH_EXPR_BINARY_OP_PLUS = AST_EXPR_BINARY_OP_PLUS,
    FLOW_GRAPH_EXPR_BINARY_OP_MINUS = AST_EXPR_BINARY_OP_MINUS,
    FLOW_GRAPH_EXPR_BINARY_OP_MULTIPLY = AST_EXPR_BINARY_OP_MULTIPLY,
    FLOW_GRAPH_EXPR_BINARY_OP_DIVIDE = AST_EXPR_BINARY_OP_DIVIDE,
    FLOW_GRAPH_EXPR_BINARY_OP_REMAINDER = AST_EXPR_BINARY_OP_REMAINDER,
    FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_AND = AST_EXPR_BINARY_OP_BITWISE_AND,
    FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_OR = AST_EXPR_BINARY_OP_BITWISE_OR,
    FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_XOR = AST_EXPR_BINARY_OP_BITWISE_XOR,
    FLOW_GRAPH_EXPR_BINARY_OP_AND = AST_EXPR_BINARY_OP_AND,
    FLOW_GRAPH_EXPR_BINARY_OP_OR = AST_EXPR_BINARY_OP_OR,
    FLOW_GRAPH_EXPR_BINARY_OP_EQ = AST_EXPR_BINARY_OP_EQ,
    FLOW_GRAPH_EXPR_BINARY_OP_NE = AST_EXPR_BINARY_OP_NE,
    FLOW_GRAPH_EXPR_BINARY_OP_LT = AST_EXPR_BINARY_OP_LT,
    FLOW_GRAPH_EXPR_BINARY_OP_LE = AST_EXPR_BINARY_OP_LE,
    FLOW_GRAPH_EXPR_BINARY_OP_GT = AST_EXPR_BINARY_OP_GT,
    FLOW_GRAPH_EXPR_BINARY_OP_GE = AST_EXPR_BINARY_OP_GE,
    FLOW_GRAPH_EXPR_BINARY_OP_LEFT_BITSHIFT = AST_EXPR_BINARY_OP_LEFT_BITSHIFT,
    FLOW_GRAPH_EXPR_BINARY_OP_RIGHT_BITSHIFT = AST_EXPR_BINARY_OP_RIGHT_BITSHIFT,
};

enum flow_graph_expr_unary_op {

    FLOW_GRAPH_EXPR_UNARY_OP_MINUS = AST_EXPR_UNARY_OP_MINUS,
    FLOW_GRAPH_EXPR_UNARY_OP_BITWISE_NOT = AST_EXPR_UNARY_OP_BITWISE_NOT,
    FLOW_GRAPH_EXPR_UNARY_OP_NOT = AST_EXPR_UNARY_OP_NOT,
};

struct flow_graph_expr;

struct flow_graph_expr_list {

    size_t size;
    size_t capacity;
    struct flow_graph_expr ** values;
};

struct flow_graph_expr_binary {

    enum flow_graph_expr_binary_op op;
    struct flow_graph_expr * lhs;
    struct flow_graph_expr * rhs;
};

struct flow_graph_expr_unary {

    enum flow_graph_expr_unary_op op;
    struct flow_graph_expr * value;
};

struct flow_graph_expr_call {

    struct flow_graph_subroutine * subroutine;
    struct flow_graph_expr_list args;
};

struct flow_graph_expr_indexer {

    struct flow_graph_expr * value;
    struct flow_graph_expr_list indices;
};

struct flow_graph_expr_local {

    struct flow_graph_local * local;
};

struct flow_graph_expr_literal {

    struct flow_graph_literal * literal;
};

struct flow_graph_expr {

    struct position position;
    struct ast_type_reference * type;

    enum flow_graph_expr_type _type;

    union {
        struct flow_graph_expr_binary binary;
        struct flow_graph_expr_unary unary;
        struct flow_graph_expr_call call;
        struct flow_graph_expr_indexer indexer;
        struct flow_graph_expr_local local;
        struct flow_graph_expr_literal literal;
    };
};

struct flow_graph_expr * flow_graph_expr_new_binary(
        struct position position,
        enum flow_graph_expr_binary_op op,
        struct flow_graph_expr * lhs,
        struct flow_graph_expr * rhs
);
struct flow_graph_expr * flow_graph_expr_new_unary(
        struct position position,
        enum flow_graph_expr_unary_op op,
        struct flow_graph_expr * value
);
struct flow_graph_expr * flow_graph_expr_new_call(struct position position, struct flow_graph_subroutine * subroutine);
struct flow_graph_expr * flow_graph_expr_new_indexer(struct position position, struct flow_graph_expr * value);
struct flow_graph_expr * flow_graph_expr_new_local(struct position position, struct flow_graph_local * local);
struct flow_graph_expr * flow_graph_expr_new_literal(struct position position, struct flow_graph_literal * literal);
void flow_graph_expr_delete(struct flow_graph_expr * expr);

struct flow_graph_expr_list flow_graph_expr_list_init(void);
void flow_graph_expr_list_append(struct flow_graph_expr_list * list, struct flow_graph_expr * value);
void flow_graph_expr_list_fini(struct flow_graph_expr_list * list);
