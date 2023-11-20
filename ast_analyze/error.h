#pragma once

#include <stddef.h>

#include "utils/position.h"


struct ast_analyze_error {

    char * message;
    const char * filename;
    struct position position;
};

struct ast_analyze_error_list {

    size_t size;
    size_t capacity;
    struct ast_analyze_error * values;
};

struct ast_analyze_error ast_analyze_error_init(char * message, const char * filename, struct position position);
void ast_analyze_error_fini(struct ast_analyze_error * error);

struct ast_analyze_error_list ast_analyze_error_list_init(void);
void ast_analyze_error_list_append(struct ast_analyze_error_list * list, struct ast_analyze_error value);
void ast_analyze_error_list_fini(struct ast_analyze_error_list * list);
