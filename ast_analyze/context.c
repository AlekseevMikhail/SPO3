#include "context.h"


struct ast_analyze_context ast_analyze_context_init_nil(void) {
    return (struct ast_analyze_context) {
        .references = ast_analyze_reference_list_init(),
        .parent = NULL,
    };
}

struct ast_analyze_context ast_analyze_context_init_cons(const struct ast_analyze_context * parent) {
    return (struct ast_analyze_context) {
        .references = ast_analyze_reference_list_init(),
        .parent = parent,
    };
}

void ast_analyze_context_fini(struct ast_analyze_context * context) {
    ast_analyze_reference_list_fini(&context->references);
    *context = (struct ast_analyze_context) { 0 };
}
