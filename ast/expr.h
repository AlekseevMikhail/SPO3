#pragma once

#include <stdlib.h>

#include "literal.h"
#include "utils/position.h"


enum ast_expr_type {

    AST_EXPR_TYPE_BINARY = 0,
    AST_EXPR_TYPE_UNARY,
    AST_EXPR_TYPE_BRACES,
    AST_EXPR_TYPE_CALL,
    AST_EXPR_TYPE_INDEXER,
    AST_EXPR_TYPE_PLACE,
    AST_EXPR_TYPE_LITERAL,
};

enum ast_expr_binary_op {

    AST_EXPR_BINARY_OP_ASSIGNMENT = 0,
    AST_EXPR_BINARY_OP_PLUS,
    AST_EXPR_BINARY_OP_MINUS,
    AST_EXPR_BINARY_OP_MULTIPLY,
    AST_EXPR_BINARY_OP_DIVIDE,
    AST_EXPR_BINARY_OP_REMAINDER,
    AST_EXPR_BINARY_OP_BITWISE_AND,
    AST_EXPR_BINARY_OP_BITWISE_OR,
    AST_EXPR_BINARY_OP_BITWISE_XOR,
    AST_EXPR_BINARY_OP_AND,
    AST_EXPR_BINARY_OP_OR,
    AST_EXPR_BINARY_OP_EQ,
    AST_EXPR_BINARY_OP_NE,
    AST_EXPR_BINARY_OP_LT,
    AST_EXPR_BINARY_OP_LE,
    AST_EXPR_BINARY_OP_GT,
    AST_EXPR_BINARY_OP_GE,
    AST_EXPR_BINARY_OP_LEFT_BITSHIFT,
    AST_EXPR_BINARY_OP_RIGHT_BITSHIFT,
    AST_EXPR_BINARY_OP_ASSIGNMENT_PLUS,
    AST_EXPR_BINARY_OP_ASSIGNMENT_MINUS,
    AST_EXPR_BINARY_OP_ASSIGNMENT_MULTIPLY,
    AST_EXPR_BINARY_OP_ASSIGNMENT_DIVIDE,
    AST_EXPR_BINARY_OP_ASSIGNMENT_REMAINDER,
    AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_AND,
    AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_OR,
    AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_XOR,
    AST_EXPR_BINARY_OP_ASSIGNMENT_AND,
    AST_EXPR_BINARY_OP_ASSIGNMENT_OR,
};

enum ast_expr_unary_op {

    AST_EXPR_UNARY_OP_MINUS = 0,
    AST_EXPR_UNARY_OP_BITWISE_NOT,
    AST_EXPR_UNARY_OP_NOT,
    AST_EXPR_UNARY_OP_INC,
    AST_EXPR_UNARY_OP_DEC,
};

struct ast_expr;

struct ast_expr_list {

    size_t size;
    size_t capacity;
    struct ast_expr ** values;
};

struct ast_expr_binary {

    enum ast_expr_binary_op op;
    struct ast_expr * lhs;
    struct ast_expr * rhs;
};

struct ast_expr_unary {

    enum ast_expr_unary_op op;
    struct ast_expr * expr;
};

struct ast_expr_braces {

    struct ast_expr * expr;
};

struct ast_expr_call {

    struct ast_expr * function;
    struct ast_expr_list arguments;
};

struct ast_expr_indexer {

    struct ast_expr * value;
    struct ast_expr_list indices;
};

struct ast_expr_place {

    char * id;
};

struct ast_expr_literal {

    struct ast_literal * value;
};

struct ast_expr {

    struct position position;

    enum ast_expr_type _type;

    union {
        struct ast_expr_binary binary;
        struct ast_expr_unary unary;
        struct ast_expr_braces braces;
        struct ast_expr_call call;
        struct ast_expr_indexer indexer;
        struct ast_expr_place place;
        struct ast_expr_literal literal;
    };
};

struct ast_expr * ast_expr_new_binary(
        struct position position,
        enum ast_expr_binary_op op,
        struct ast_expr * lhs,
        struct ast_expr * rhs
);
struct ast_expr * ast_expr_new_unary(struct position position, enum ast_expr_unary_op op, struct ast_expr * expr);
struct ast_expr * ast_expr_new_braces(struct position position, struct ast_expr * expr);
struct ast_expr * ast_expr_new_call(struct position position, struct ast_expr * function, struct ast_expr_list arguments);
struct ast_expr * ast_expr_new_indexer(struct position position, struct ast_expr * value, struct ast_expr_list indices);
struct ast_expr * ast_expr_new_place(struct position position, char * id);
struct ast_expr * ast_expr_new_literal(struct position position, struct ast_literal * literal);
void ast_expr_delete(struct ast_expr * value);

struct ast_expr_list ast_expr_list_init(void);
void ast_expr_list_append(struct ast_expr_list * list, struct ast_expr * value);
void ast_expr_list_fini(struct ast_expr_list * list);
