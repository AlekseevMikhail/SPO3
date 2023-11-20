#include "subroutine.h"

#include "utils/mallocs.h"


struct flow_graph_subroutine * flow_graph_subroutine_new(char * id, char * filename, bool defined) {
    struct flow_graph_subroutine * result = mallocs(sizeof(struct flow_graph_subroutine));

    result->id = id;
    result->filename = filename;
    result->defined = defined;

    result->args_num = 0;
    result->return_type = NULL;

    result->locals = flow_graph_local_list_init();
    result->nodes = flow_graph_node_list_init();

    return result;
}

void flow_graph_subroutine_delete(struct flow_graph_subroutine * subroutine) {
    if (!subroutine) {
        return;
    }

    free(subroutine->id);
    free(subroutine->filename);

    ast_type_reference_delete(subroutine->return_type);

    flow_graph_local_list_fini(&subroutine->locals);
    flow_graph_node_list_fini(&subroutine->nodes);

    free(subroutine);
}

struct flow_graph_subroutine_list flow_graph_subroutine_list_init(void) {
    return (struct flow_graph_subroutine_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct flow_graph_subroutine *)),
    };
}

void flow_graph_subroutine_list_append(struct flow_graph_subroutine_list * list, struct flow_graph_subroutine * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct flow_graph_subroutine ** const new_values =
                reallocs(list->values, sizeof(struct flow_graph_subroutine *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void flow_graph_subroutine_list_fini(struct flow_graph_subroutine_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        flow_graph_subroutine_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct flow_graph_subroutine_list) { 0 };
}
