#include "source_item.h"

#include "utils/mallocs.h"


struct ast_source_item * ast_source_item_new_func_decl(
        struct position position,
        struct ast_function_signature * signature,
        struct ast_stmt * body
) {
    struct ast_source_item * result = mallocs(sizeof(struct ast_source_item));

    result->_type = AST_SOURCE_ITEM_TYPE_FUNC_DECL;
    result->position = position;
    result->func_decl = (struct ast_source_item_func_decl) {
        .signature = signature,
        .body = body,
    };

    return result;
}

void ast_source_item_delete(struct ast_source_item * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case AST_SOURCE_ITEM_TYPE_FUNC_DECL:
            ast_function_signature_delete(value->func_decl.signature);
            ast_stmt_delete(value->func_decl.body);
            break;
    }

    free(value);
}

struct ast_source_item_list ast_source_item_list_init(void) {
    return (struct ast_source_item_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_source_item *)),
    };
}

void ast_source_item_list_append(struct ast_source_item_list * list, struct ast_source_item * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_source_item ** const new_values =
                reallocs(list->values, sizeof(struct ast_source_item *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_source_item_list_fini(struct ast_source_item_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_source_item_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct ast_source_item_list) { 0 };
}
