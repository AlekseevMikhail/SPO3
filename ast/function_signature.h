#pragma once

#include <stdlib.h>

#include "type_reference.h"
#include "utils/position.h"


struct ast_function_signature_arg {

    struct ast_type_reference * type;
    char * id;

    struct position position;
};

struct ast_function_signature_arg_list {

    size_t size;
    size_t capacity;
    struct ast_function_signature_arg * values;
};

struct ast_function_signature {

    struct ast_type_reference * return_type;
    char * id;

    struct ast_function_signature_arg_list args;

    struct position position;
};

struct ast_function_signature_arg ast_function_signature_arg_init(
        struct position position,
        struct ast_type_reference * type,
        char * id
);
void ast_function_signature_arg_fini(struct ast_function_signature_arg * value);

struct ast_function_signature_arg_list ast_function_signature_arg_list_init(void);
void ast_function_signature_arg_list_append(
        struct ast_function_signature_arg_list * list,
        struct ast_function_signature_arg value
);
void ast_function_signature_arg_list_fini(struct ast_function_signature_arg_list * list);

struct ast_function_signature * ast_function_signature_new(
        struct position position,
        struct ast_type_reference * return_type,
        char * id,
        struct ast_function_signature_arg_list args
);
void ast_function_signature_delete(struct ast_function_signature * value);
