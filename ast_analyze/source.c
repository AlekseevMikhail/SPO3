#include "source.h"

#include <stdlib.h>

#include "utils/mallocs.h"


struct ast_analyze_source ast_analyze_source_init(const char * filename, struct ast_source * source) {
    return (struct ast_analyze_source) {
        .filename = filename,
        .source = source,
    };
}

void ast_analyze_source_fini(struct ast_analyze_source * source) {
    // ничем не владеем, ничего не делаем

    *source = (struct ast_analyze_source) { 0 };
}

struct ast_analyze_source_list ast_analyze_source_list_init(void) {
    return (struct ast_analyze_source_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_analyze_source)),
    };
}

void ast_analyze_source_list_append(struct ast_analyze_source_list * list, struct ast_analyze_source value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_analyze_source * const new_values =
                reallocs(list->values, sizeof(struct ast_analyze_source) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_analyze_source_list_fini(struct ast_analyze_source_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_analyze_source_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct ast_analyze_source_list) { 0 };
}
