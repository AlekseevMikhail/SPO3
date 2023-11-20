#include "error.h"

#include "utils/mallocs.h"


struct ast_analyze_error ast_analyze_error_init(char * message, const char * filename, struct position position) {
    return (struct ast_analyze_error) {
        .message = message,
        .filename = filename,
        .position = position,
    };
}

void ast_analyze_error_fini(struct ast_analyze_error * error) {
    free(error->message);
    error->message = NULL;
}

struct ast_analyze_error_list ast_analyze_error_list_init(void) {
    return (struct ast_analyze_error_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_analyze_error)),
    };
}

void ast_analyze_error_list_append(struct ast_analyze_error_list * list, struct ast_analyze_error value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_analyze_error * const new_values =
                reallocs(list->values, sizeof(struct ast_analyze_error) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_analyze_error_list_fini(struct ast_analyze_error_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_analyze_error_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct ast_analyze_error_list) { 0 };
}
