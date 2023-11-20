#include "literal.h"

#include "utils/mallocs.h"


struct flow_graph_literal * flow_graph_literal_new_bool(struct position position, bool value) {
    struct flow_graph_literal * const result = mallocs(sizeof(struct flow_graph_literal));

    result->position = position;
    result->_type = FLOW_GRAPH_LITERAL_TYPE_BOOL;
    result->_bool.value = value;

    return result;
}

struct flow_graph_literal * flow_graph_literal_new_str(struct position position, char * value) {
    struct flow_graph_literal * const result = mallocs(sizeof(struct flow_graph_literal));

    result->position = position;
    result->_type = FLOW_GRAPH_LITERAL_TYPE_STR;
    result->str.value = value;

    return result;
}

struct flow_graph_literal * flow_graph_literal_new_char(struct position position, char value) {
    struct flow_graph_literal * const result = mallocs(sizeof(struct flow_graph_literal));

    result->position = position;
    result->_type = FLOW_GRAPH_LITERAL_TYPE_CHAR;
    result->_char.value = value;

    return result;
}

struct flow_graph_literal * flow_graph_literal_new_int(struct position position, uint64_t value) {
    struct flow_graph_literal * const result = mallocs(sizeof(struct flow_graph_literal));

    result->position = position;
    result->_type = FLOW_GRAPH_LITERAL_TYPE_INT;
    result->_int.value = value;

    return result;
}

void flow_graph_literal_delete(struct flow_graph_literal * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case FLOW_GRAPH_LITERAL_TYPE_BOOL:
        case FLOW_GRAPH_LITERAL_TYPE_CHAR:
        case FLOW_GRAPH_LITERAL_TYPE_INT:
            break;

        case FLOW_GRAPH_LITERAL_TYPE_STR:
            free(value->str.value);
            break;
    }

    free(value);
}
