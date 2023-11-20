#include "function_signature.h"

#include "utils/mallocs.h"


struct ast_function_signature_arg ast_function_signature_arg_init(
        struct position position,
        struct ast_type_reference * type,
        char * id
) {
    return (struct ast_function_signature_arg) {
        .type = type,
        .id = id,
        .position = position,
    };
}

void ast_function_signature_arg_fini(struct ast_function_signature_arg * value) {
    ast_type_reference_delete(value->type);
    free(value->id);

    *value = (struct ast_function_signature_arg) { 0 };
}

struct ast_function_signature_arg_list ast_function_signature_arg_list_init(void) {
    return (struct ast_function_signature_arg_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_function_signature_arg)),
    };
}

void ast_function_signature_arg_list_append(
        struct ast_function_signature_arg_list * list,
        struct ast_function_signature_arg value
) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_function_signature_arg * const new_values =
                reallocs(list->values, sizeof(struct ast_function_signature_arg) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_function_signature_arg_list_fini(struct ast_function_signature_arg_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_function_signature_arg_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct ast_function_signature_arg_list) { 0 };
}

struct ast_function_signature * ast_function_signature_new(
        struct position position,
        struct ast_type_reference * return_type,
        char * id,
        struct ast_function_signature_arg_list args
) {
    struct ast_function_signature * result = mallocs(sizeof(struct ast_function_signature));

    result->return_type = return_type;
    result->id = id;
    result->args = args;
    result->position = position;

    return result;
}

void ast_function_signature_delete(struct ast_function_signature * value) {
    if (!value) {
        return;
    }

    ast_type_reference_delete(value->return_type);
    ast_function_signature_arg_list_fini(&value->args);
    free(value->id);

    free(value);
}
