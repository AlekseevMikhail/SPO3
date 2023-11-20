#include "stmt.h"

#include "utils/mallocs.h"


struct ast_stmt_var_id ast_stmt_var_id_init(struct position position, char * id, struct ast_expr * value) {
    return (struct ast_stmt_var_id) {
        .id = id,
        .value = value,
        .position = position,
    };
}

void ast_stmt_var_id_fini(struct ast_stmt_var_id * value) {
    free(value->id);
    ast_expr_delete(value->value);
}

struct ast_stmt_var_id_list ast_stmt_var_id_list_init(void) {
    return (struct ast_stmt_var_id_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_stmt_var_id)),
    };
}

void ast_stmt_var_id_list_append(struct ast_stmt_var_id_list * list, struct ast_stmt_var_id value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_stmt_var_id * const new_values =
                reallocs(list->values, sizeof(struct ast_stmt_var_id) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_stmt_var_id_list_fini(struct ast_stmt_var_id_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_stmt_var_id_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct ast_stmt_var_id_list) { 0 };
}

struct ast_stmt * ast_stmt_new_var(
        struct position position,
        struct ast_type_reference * type,
        struct ast_stmt_var_id_list ids
) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_VAR;
    result->position = position;
    result->var = (struct ast_stmt_var) {
        .type = type,
        .ids = ids,
    };

    return result;
}

struct ast_stmt * ast_stmt_new_if(
        struct position position,
        struct ast_expr * condition,
        struct ast_stmt * then_branch,
        struct ast_stmt * else_branch
) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_IF;
    result->position = position;
    result->_if = (struct ast_stmt_if) {
        .condition = condition,
        .then_branch = then_branch,
        .else_branch = else_branch,
    };

    return result;
}

struct ast_stmt * ast_stmt_new_block(struct position position, struct ast_stmt_list stmts) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_BLOCK;
    result->position = position;
    result->block = (struct ast_stmt_block) {
        .stmts = stmts,
    };

    return result;
}

struct ast_stmt * ast_stmt_new_while(struct position position, struct ast_expr * condition, struct ast_stmt * body) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_WHILE;
    result->position = position;
    result->_while = (struct ast_stmt_while) {
        .condition = condition,
        .body = body,
    };

    return result;
}

struct ast_stmt * ast_stmt_new_do(struct position position, struct ast_stmt * body, struct ast_expr * condition) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_DO;
    result->position = position;
    result->_do = (struct ast_stmt_do) {
        .body = body,
        .condition = condition,
    };

    return result;
}

struct ast_stmt * ast_stmt_new_break(struct position position) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_BREAK;
    result->position = position;

    return result;
}

struct ast_stmt * ast_stmt_new_expr(struct position position, struct ast_expr * expr) {
    struct ast_stmt * result = mallocs(sizeof(struct ast_stmt));

    result->_type = AST_STMT_TYPE_EXPR;
    result->position = position;
    result->expr = (struct ast_stmt_expr) {
        .expr = expr,
    };

    return result;
}

void ast_stmt_delete(struct ast_stmt * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case AST_STMT_TYPE_VAR:
            ast_type_reference_delete(value->var.type);
            ast_stmt_var_id_list_fini(&value->var.ids);
            break;

        case AST_STMT_TYPE_IF:
            ast_expr_delete(value->_if.condition);
            ast_stmt_delete(value->_if.then_branch);
            ast_stmt_delete(value->_if.else_branch);
            break;

        case AST_STMT_TYPE_BLOCK:
            ast_stmt_list_fini(&value->block.stmts);
            break;

        case AST_STMT_TYPE_WHILE:
            ast_expr_delete(value->_while.condition);
            ast_stmt_delete(value->_while.body);
            break;

        case AST_STMT_TYPE_DO:
            ast_stmt_delete(value->_while.body);
            ast_expr_delete(value->_while.condition);
            break;

        case AST_STMT_TYPE_BREAK:
            break;

        case AST_STMT_TYPE_EXPR:
            ast_expr_delete(value->expr.expr);
            break;
    }

    free(value);
}

struct ast_stmt_list ast_stmt_list_init(void) {
    return (struct ast_stmt_list) {
        .size = 0,
        .capacity = 1,
        .values = mallocs(sizeof(struct ast_stmt *)),
    };
}

void ast_stmt_list_append(struct ast_stmt_list * list, struct ast_stmt * value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct ast_stmt ** const new_values = reallocs(list->values, sizeof(struct ast_stmt *) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void ast_stmt_list_fini(struct ast_stmt_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        ast_stmt_delete(list->values[i]);
    }

    free(list->values);
    *list = (struct ast_stmt_list) { 0 };
}
