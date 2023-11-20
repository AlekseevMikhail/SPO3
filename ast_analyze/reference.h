#pragma once

#include <stdlib.h>

#include "flow_graph.h"


enum ast_analyze_reference_type {

    AST_ANALYZE_REFERENCE_TYPE_LOCAL = 0,
    AST_ANALYZE_REFERENCE_TYPE_GLOBAL
};

struct ast_analyze_reference_local {

    struct flow_graph_local * local;
};

struct ast_analyze_reference_global {

    struct flow_graph_subroutine * subroutine;
};

struct ast_analyze_reference {

    const char * id;
    enum ast_analyze_reference_type _type;

    union {
        struct ast_analyze_reference_local local;
        struct ast_analyze_reference_global global;
    };
};

struct ast_analyze_reference_list {

    size_t size;
    size_t capacity;
    struct ast_analyze_reference * values;
};

struct ast_analyze_reference ast_analyze_reference_init_local(const char * id, struct flow_graph_local * local);
struct ast_analyze_reference ast_analyze_reference_init_global(
        const char * id,
        struct flow_graph_subroutine * subroutine
);
void ast_analyze_reference_fini(struct ast_analyze_reference * reference);

struct ast_analyze_reference_list ast_analyze_reference_list_init(void);
void ast_analyze_reference_list_append(
        struct ast_analyze_reference_list * list,
        struct ast_analyze_reference value
);
void ast_analyze_reference_list_fini(struct ast_analyze_reference_list * list);
