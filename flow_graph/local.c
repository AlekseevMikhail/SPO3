#include "local.h"

#include "utils/mallocs.h"


struct flow_graph_local * flow_graph_local_new(char * id, struct ast_type_reference * type, struct position position) {
    struct flow_graph_local * const result = mallocs(sizeof(struct flow_graph_local));

    result->id = id;
    result->type = type;

    result->index = 0;
    result->position = position;

    return result;
}

void flow_graph_local_delete(struct flow_graph_local * local) {
    if (!local) {
        return;
    }

    free(local->id);
    ast_type_reference_delete(local->type);

    free(local);
}

struct flow_graph_local_list flow_graph_local_list_init(void) {
    return (struct flow_graph_local_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct flow_graph_local *)),
    };
}

void flow_graph_local_list_append(struct flow_graph_local_list * list, struct flow_graph_local * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct flow_graph_local ** const new_values =
                reallocs(list->values, sizeof(struct flow_graph_local *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void flow_graph_local_list_fini(struct flow_graph_local_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        flow_graph_local_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct flow_graph_local_list) { 0 };
}
