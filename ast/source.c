#include "source.h"

#include "utils/mallocs.h"


struct ast_source * ast_source_new(struct position position) {
    struct ast_source * result = mallocs(sizeof(struct ast_source));

    result->items = ast_source_item_list_init();
    result->position = position;

    return result;
}

void ast_source_delete(struct ast_source * value) {
    if (!value) {
        return;
    }

    ast_source_item_list_fini(&value->items);

    free(value);
}
