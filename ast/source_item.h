#pragma once

#include <stdlib.h>

#include "function_signature.h"
#include "stmt.h"
#include "utils/position.h"


enum ast_source_item_type {

    AST_SOURCE_ITEM_TYPE_FUNC_DECL = 0,
};

struct ast_source_item_func_decl {

    struct ast_function_signature * signature;
    struct ast_stmt * body;
};

struct ast_source_item {

    enum ast_source_item_type _type;
    struct position position;

    union {
        struct ast_source_item_func_decl func_decl;
    };
};

struct ast_source_item_list {

    size_t size;
    size_t capacity;
    struct ast_source_item ** values;
};

struct ast_source_item * ast_source_item_new_func_decl(
        struct position position,
        struct ast_function_signature * signature,
        struct ast_stmt * body
);
void ast_source_item_delete(struct ast_source_item * value);

struct ast_source_item_list ast_source_item_list_init(void);
void ast_source_item_list_append(struct ast_source_item_list * list, struct ast_source_item * value);
void ast_source_item_list_fini(struct ast_source_item_list * list);
