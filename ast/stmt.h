#pragma once

#include <stdlib.h>

#include "type_reference.h"
#include "expr.h"
#include "utils/position.h"


enum ast_stmt_type {

    AST_STMT_TYPE_VAR = 0,
    AST_STMT_TYPE_IF,
    AST_STMT_TYPE_BLOCK,
    AST_STMT_TYPE_WHILE,
    AST_STMT_TYPE_DO,
    AST_STMT_TYPE_BREAK,
    AST_STMT_TYPE_EXPR,
};

struct ast_stmt;

struct ast_stmt_list {

    size_t size;
    size_t capacity;
    struct ast_stmt ** values;
};

struct ast_stmt_var_id {

    char * id;
    struct ast_expr * value;

    struct position position;
};

struct ast_stmt_var_id_list {

    size_t size;
    size_t capacity;
    struct ast_stmt_var_id * values;
};

struct ast_stmt_var {

    struct ast_type_reference * type;
    struct ast_stmt_var_id_list ids;
};

struct ast_stmt_if {

    struct ast_expr * condition;
    struct ast_stmt * then_branch;
    struct ast_stmt * else_branch;
};

struct ast_stmt_block {

    struct ast_stmt_list stmts;
};

struct ast_stmt_while {

    struct ast_expr * condition;
    struct ast_stmt * body;
};

struct ast_stmt_do {

    struct ast_stmt * body;
    struct ast_expr * condition;
};

struct ast_stmt_expr {

    struct ast_expr * expr;
};

struct ast_stmt {

    enum ast_stmt_type _type;
    struct position position;

    union {
        struct ast_stmt_var var;
        struct ast_stmt_if _if;
        struct ast_stmt_block block;
        struct ast_stmt_while _while;
        struct ast_stmt_do _do;
        struct ast_stmt_expr expr;
    };
};

struct ast_stmt_var_id ast_stmt_var_id_init(struct position position, char * id, struct ast_expr * value);
void ast_stmt_var_id_fini(struct ast_stmt_var_id * value);

struct ast_stmt_var_id_list ast_stmt_var_id_list_init(void);
void ast_stmt_var_id_list_append(struct ast_stmt_var_id_list * list, struct ast_stmt_var_id value);
void ast_stmt_var_id_list_fini(struct ast_stmt_var_id_list * list);

struct ast_stmt * ast_stmt_new_var(
        struct position position,
        struct ast_type_reference * type,
        struct ast_stmt_var_id_list ids
);
struct ast_stmt * ast_stmt_new_if(
        struct position position,
        struct ast_expr * condition,
        struct ast_stmt * then_branch,
        struct ast_stmt * else_branch
);
struct ast_stmt * ast_stmt_new_block(struct position position, struct ast_stmt_list stmts);
struct ast_stmt * ast_stmt_new_while(struct position position, struct ast_expr * condition, struct ast_stmt * body);
struct ast_stmt * ast_stmt_new_do(struct position position, struct ast_stmt * body, struct ast_expr * condition);
struct ast_stmt * ast_stmt_new_break(struct position position);
struct ast_stmt * ast_stmt_new_expr(struct position position, struct ast_expr * expr);
void ast_stmt_delete(struct ast_stmt * value);

struct ast_stmt_list ast_stmt_list_init(void);
void ast_stmt_list_append(struct ast_stmt_list * list, struct ast_stmt * value);
void ast_stmt_list_fini(struct ast_stmt_list * list);
