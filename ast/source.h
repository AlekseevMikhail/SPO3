#pragma once

#include "source_item.h"
#include "utils/position.h"


struct ast_source {

    struct ast_source_item_list items;
    struct position position;
};

struct ast_source * ast_source_new(struct position position);
void ast_source_delete(struct ast_source * value);
