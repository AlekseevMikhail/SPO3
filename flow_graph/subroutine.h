#pragma once

#include <stdbool.h>

#include "ast.h"
#include "local.h"
#include "node.h"


struct flow_graph_subroutine {

    char * id;
    char * filename;
    bool defined;

    size_t args_num;
    struct ast_type_reference * return_type;
    struct position position;

    struct flow_graph_local_list locals;
    struct flow_graph_node_list nodes;
};

struct flow_graph_subroutine_list {

    size_t size;
    size_t capacity;
    struct flow_graph_subroutine ** values;
};

struct flow_graph_subroutine * flow_graph_subroutine_new(char * id, char * filename, bool defined);
void flow_graph_subroutine_delete(struct flow_graph_subroutine * subroutine);

struct flow_graph_subroutine_list flow_graph_subroutine_list_init(void);
void flow_graph_subroutine_list_append(struct flow_graph_subroutine_list * list, struct flow_graph_subroutine * value);
void flow_graph_subroutine_list_fini(struct flow_graph_subroutine_list * list);
