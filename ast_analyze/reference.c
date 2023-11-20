#include "reference.h"

#include "utils/mallocs.h"


struct ast_analyze_reference ast_analyze_reference_init_local(const char * id, struct flow_graph_local * local) {
    return (struct ast_analyze_reference) {
        .id = id,
        ._type = AST_ANALYZE_REFERENCE_TYPE_LOCAL,
        .local = (struct ast_analyze_reference_local) {
            .local = local,
        },
    };
}

struct ast_analyze_reference ast_analyze_reference_init_global(
        const char * id,
        struct flow_graph_subroutine * subroutine
) {
    return (struct ast_analyze_reference) {
        .id = id,
        ._type = AST_ANALYZE_REFERENCE_TYPE_GLOBAL,
        .global = (struct ast_analyze_reference_global) {
            .subroutine = subroutine,
        },
    };
}

void ast_analyze_reference_fini(struct ast_analyze_reference * reference) {
    // референсы не владеют тем на что указывают, поэтому ничего не очищаем

    *reference = (struct ast_analyze_reference) { 0 };
}

struct ast_analyze_reference_list ast_analyze_reference_list_init(void) {
    return (struct ast_analyze_reference_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_analyze_reference)),
    };
}

void ast_analyze_reference_list_append(
        struct ast_analyze_reference_list * list,
        struct ast_analyze_reference value
) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_analyze_reference * const new_values =
                reallocs(list->values, sizeof(struct ast_analyze_reference) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_analyze_reference_list_fini(struct ast_analyze_reference_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_analyze_reference_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct ast_analyze_reference_list) { 0 };
}
