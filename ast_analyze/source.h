#pragma once

#include <stddef.h>


struct ast_analyze_source {

    const char * filename;
    struct ast_source * source;
};

struct ast_analyze_source_list {

    size_t size;
    size_t capacity;
    struct ast_analyze_source * values;
};

struct ast_analyze_source ast_analyze_source_init(const char * filename, struct ast_source * source);
void ast_analyze_source_fini(struct ast_analyze_source * source);

struct ast_analyze_source_list ast_analyze_source_list_init(void);
void ast_analyze_source_list_append(struct ast_analyze_source_list * list, struct ast_analyze_source value);
void ast_analyze_source_list_fini(struct ast_analyze_source_list * list);
