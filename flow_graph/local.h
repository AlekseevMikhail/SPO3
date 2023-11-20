#pragma once

#include <stddef.h>
#include "ast/type_reference.h"


struct flow_graph_local {

    char * id;
    struct ast_type_reference * type;

    size_t index;
    struct position position;
};

struct flow_graph_local_list {

    size_t size;
    size_t capacity;
    struct flow_graph_local ** values;
};

struct flow_graph_local * flow_graph_local_new(char * id, struct ast_type_reference * type, struct position position);
void flow_graph_local_delete(struct flow_graph_local * local);

struct flow_graph_local_list flow_graph_local_list_init(void);
void flow_graph_local_list_append(struct flow_graph_local_list * list, struct flow_graph_local * value);
void flow_graph_local_list_fini(struct flow_graph_local_list * list);
