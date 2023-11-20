#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "utils/position.h"


enum flow_graph_literal_type {

    FLOW_GRAPH_LITERAL_TYPE_BOOL = 0,
    FLOW_GRAPH_LITERAL_TYPE_STR,
    FLOW_GRAPH_LITERAL_TYPE_CHAR,
    FLOW_GRAPH_LITERAL_TYPE_INT,
};

struct flow_graph_literal_bool {

    bool value;
};

struct flow_graph_literal_str {

    char * value;
};

struct flow_graph_literal_char {

    char value;
};

struct flow_graph_literal_int {

    uint64_t value;
};

struct flow_graph_literal {

    enum flow_graph_literal_type _type;
    struct position position;

    union {
        struct flow_graph_literal_bool _bool;
        struct flow_graph_literal_str str;
        struct flow_graph_literal_char _char;
        struct flow_graph_literal_int _int;
    };
};

struct flow_graph_literal * flow_graph_literal_new_bool(struct position position, bool value);
struct flow_graph_literal * flow_graph_literal_new_str(struct position position, char * value);
struct flow_graph_literal * flow_graph_literal_new_char(struct position position, char value);
struct flow_graph_literal * flow_graph_literal_new_int(struct position position, uint64_t value);
void flow_graph_literal_delete(struct flow_graph_literal * value);
