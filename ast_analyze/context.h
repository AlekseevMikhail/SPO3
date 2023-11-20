#pragma once

#include "reference.h"


struct ast_analyze_context {

    struct ast_analyze_reference_list references;
    const struct ast_analyze_context * parent;
};

struct ast_analyze_context ast_analyze_context_init_nil(void);
struct ast_analyze_context ast_analyze_context_init_cons(const struct ast_analyze_context * parent);
void ast_analyze_context_fini(struct ast_analyze_context * context);
