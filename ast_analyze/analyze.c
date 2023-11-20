#include "analyze.h"

#include <string.h>
#include <assert.h>
#include <errno.h>

#include "context.h"
#include "flow_graph.h"
#include "utils/unreachable.h"
#include "utils/mallocs.h"


static void raise_error(
        const char * message,
        const char * filename,
        struct position position,
        struct ast_analyze_error_list * errors
) {
    ast_analyze_error_list_append(errors, ast_analyze_error_init(strdup(message), filename, position));
}

static struct flow_graph_local * append_local(
        struct flow_graph_local * local,
        struct flow_graph_subroutine * subroutine
) {
    flow_graph_local_list_append(&subroutine->locals, local);
    local->index = subroutine->locals.size;
    return local;
}

static struct flow_graph_subroutine * append_subroutine(
        const char * filename,
        struct position position,
        const struct ast_source_item_func_decl * func_decl,
        struct flow_graph_subroutine_list * list,
        struct ast_analyze_error_list * errors
) {
    struct flow_graph_subroutine * result;

    for (size_t i = 0; i < list->size; ++i) {
        if (strcmp(list->values[i]->id, func_decl->signature->id) == 0) {
            result = list->values[i];
            goto found;
        }
    }

    result = flow_graph_subroutine_new(
            strdup(func_decl->signature->id),
            strdup(filename),
            func_decl->body != NULL
    );

    result->args_num = func_decl->signature->args.size;
    result->return_type = ast_type_reference_clone(func_decl->signature->return_type);
    result->position = position;

    for (size_t i = 0; i < func_decl->signature->args.size; ++i) {
        struct ast_function_signature_arg * const arg = &func_decl->signature->args.values[i];

        append_local(
                flow_graph_local_new(strdup(arg->id), ast_type_reference_clone(arg->type), arg->position),
                result
        );
    }

    flow_graph_subroutine_list_append(list, result);
    return result;

found:
    if (result->defined && func_decl->body != NULL) {
        raise_error("double function definition", filename, func_decl->signature->position, errors);
        return result;
    }

    bool valid = true;

    valid &= result->args_num == func_decl->signature->args.size;

    if (result->return_type && func_decl->signature->return_type) {
        valid &= ast_type_reference_equals(result->return_type, func_decl->signature->return_type);
    } else if (func_decl->signature->return_type) {
        result->return_type = ast_type_reference_clone(func_decl->signature->return_type);
    }

    for (size_t i = 0; i < result->args_num; ++i) {
        struct flow_graph_local * const local = result->locals.values[i];
        struct ast_function_signature_arg * const arg = &func_decl->signature->args.values[i];

        if (!result->defined) {
            // сохраняем имя параметра функции, которое предоставляет определение, если оно есть
            // если в других объявлениях имя другое, не обращаем внимание

            free(local->id);
            local->id = strdup(arg->id);
            local->position = arg->position;
        }

        if (local->type && arg->type) {
            valid &= ast_type_reference_equals(local->type, arg->type);
        } else if (arg->type) {
            local->type = ast_type_reference_clone(arg->type);
        }
    }

    if (!valid) {
        raise_error(
                "function declared with conflicting signature",
                filename,
                func_decl->signature->position,
                errors
        );
    } else if (!result->defined) {
        // считаем, что если функция не была ещё определена,
        // а данное объявление валидно, то оно может предоставить определение
        result->defined = func_decl->body != NULL;

        // сохраняем имя файла определения приоритетно
        free(result->filename);
        result->filename = strdup(filename);

        result->position = position;
    }

    return result;
}

static struct ast_analyze_context create_global_context(
        const struct ast_analyze_source_list * sources,
        struct flow_graph_subroutine_list * subroutines,
        struct ast_analyze_error_list * errors
) {
    struct ast_analyze_context result = ast_analyze_context_init_nil();

    for (size_t i = 0; i < sources->size; ++i) {
        const char * const filename = sources->values[i].filename;
        const struct ast_source * const source = sources->values[i].source;

        for (size_t j = 0; j < source->items.size; ++j) {
            const struct ast_source_item * const item = source->items.values[j];

            switch (item->_type) {
                case AST_SOURCE_ITEM_TYPE_FUNC_DECL: {
                    struct flow_graph_subroutine * subroutine =
                            append_subroutine(filename, item->position, &item->func_decl, subroutines, errors);

                    ast_analyze_reference_list_append(
                            &result.references,
                            ast_analyze_reference_init_global(item->func_decl.signature->id, subroutine)
                    );
                    break;
                }
            }
        }
    }

    return result;
}

static char * parse_str_literal(
        const char * filename,
        struct position position,
        const char * str,
        struct ast_analyze_error_list * errors
) {
    char * const result = mallocs(sizeof(char) * strlen(str));

    bool escaped = false;
    for (size_t i = 1, j = 0; str[i]; ++i) {
        if (escaped) {
            char c = '\0';

            switch (str[i]) {
                case 'b':
                    c = '\b';
                    break;

                case 'n':
                    c = '\n';
                    break;

                case 'r':
                    c = '\r';
                    break;

                case 't':
                    c = '\t';
                    break;

                case '\\':
                    c = '\\';
                    break;

                case '"':
                    c = '"';
                    break;

                default:
                    raise_error("unknown escape sequence", filename, position, errors);
            }

            result[j] = c;
            ++j;

            escaped = false;
            continue;
        }

        if (str[i] == '\\') {
            escaped = true;
            continue;
        }

        if (str[i] == '"') {
            result[j] = '\0';
            break;
        }

        result[j] = str[i];
        ++j;
    }

    return reallocs(result, sizeof(char) * (strlen(result) + 1));
}

static struct flow_graph_literal * parse_numeric_literal(
        const char * filename,
        struct position position,
        const char * str,
        int base,
        struct ast_analyze_error_list * errors
) {
    errno = 0;
    uint64_t result = strtoull(str, NULL, base);

    if (errno) {
        raise_error("value is not representable as 64-bit number", filename, position, errors);
    }

    return flow_graph_literal_new_int(position, result);
}

static struct flow_graph_literal * analyze_literal(
        const char * filename,
        struct ast_literal * literal,
        struct ast_analyze_error_list * errors
) {
    if (!literal) {
        return NULL;
    }

    switch (literal->_type) {
        case AST_LITERAL_TYPE_BOOL:
            return flow_graph_literal_new_bool(literal->position, literal->_bool.value);

        case AST_LITERAL_TYPE_STR:
            return flow_graph_literal_new_str(
                    literal->position,
                    parse_str_literal(filename, literal->position, literal->str.value, errors)
            );

        case AST_LITERAL_TYPE_CHAR:
            return flow_graph_literal_new_char(literal->position, literal->_char.value);

        case AST_LITERAL_TYPE_HEX:
            return parse_numeric_literal(filename, literal->position, literal->hex.value + 2, 16, errors);

        case AST_LITERAL_TYPE_BITS:
            return parse_numeric_literal(filename, literal->position, literal->bits.value + 2, 2, errors);

        case AST_LITERAL_TYPE_DEC:
            return parse_numeric_literal(filename, literal->position, literal->dec.value, 10, errors);
    }

    unreachable();
}

static const struct ast_analyze_reference * lookup_context(
        const char * filename,
        struct position position,
        const struct ast_analyze_context * context,
        const char * id,
        struct ast_analyze_error_list * errors
) {
    while (context) {
        for (size_t i = 0; i < context->references.size; ++i) {
            if (strcmp(context->references.values[i].id, id) == 0) {
                return &context->references.values[i];
            }
        }

        context = context->parent;
    }

    const size_t msg_size = 27 + strlen(id);
    char * const msg = mallocs(sizeof(char) * msg_size);
    snprintf(msg, msg_size, "undefined reference to \"%s\"", id);
    raise_error(msg, filename, position, errors);
    free(msg);

    return NULL;
}

static struct flow_graph_expr * default_expr(const struct ast_expr * expr) {
    return flow_graph_expr_new_literal(expr->position, flow_graph_literal_new_int(expr->position, 0));
}

static struct flow_graph_expr * check_assignable(
        const struct flow_graph_subroutine * subroutine,
        struct flow_graph_expr * expr,
        struct ast_analyze_error_list * errors
) {
    if (!expr) {
        return NULL;
    }

    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
        case FLOW_GRAPH_EXPR_TYPE_LOCAL:
            break;

        default:
            raise_error("left operand is not assignable", subroutine->filename, expr->position, errors);
            break;
    }

    return expr;
}

static struct flow_graph_expr * analyze_expr(
        const struct ast_analyze_context * context,
        const struct flow_graph_subroutine * subroutine,
        const struct ast_expr * expr,
        struct ast_analyze_error_list * errors
) {
    if (!expr) {
        return NULL;
    }

    switch (expr->_type) {
        case AST_EXPR_TYPE_BINARY: {
            bool assignment;
            enum flow_graph_expr_binary_op op;

            switch (expr->binary.op) {
                case AST_EXPR_BINARY_OP_ASSIGNMENT:
                    return flow_graph_expr_new_binary(
                            expr->position,
                            FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT,
                            check_assignable(subroutine, analyze_expr(context, subroutine, expr->binary.lhs, errors), errors),
                            analyze_expr(context, subroutine, expr->binary.rhs, errors)
                    );

                case AST_EXPR_BINARY_OP_PLUS:
                case AST_EXPR_BINARY_OP_MINUS:
                case AST_EXPR_BINARY_OP_MULTIPLY:
                case AST_EXPR_BINARY_OP_DIVIDE:
                case AST_EXPR_BINARY_OP_REMAINDER:
                case AST_EXPR_BINARY_OP_BITWISE_AND:
                case AST_EXPR_BINARY_OP_BITWISE_OR:
                case AST_EXPR_BINARY_OP_BITWISE_XOR:
                case AST_EXPR_BINARY_OP_AND:
                case AST_EXPR_BINARY_OP_OR:
                case AST_EXPR_BINARY_OP_EQ:
                case AST_EXPR_BINARY_OP_NE:
                case AST_EXPR_BINARY_OP_LT:
                case AST_EXPR_BINARY_OP_LE:
                case AST_EXPR_BINARY_OP_GT:
                case AST_EXPR_BINARY_OP_GE:
                case AST_EXPR_BINARY_OP_LEFT_BITSHIFT:
                case AST_EXPR_BINARY_OP_RIGHT_BITSHIFT:
                    assignment = false;
                    op = (enum flow_graph_expr_binary_op) expr->binary.op;
                    break;

                case AST_EXPR_BINARY_OP_ASSIGNMENT_PLUS:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_MINUS:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_MULTIPLY:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_DIVIDE:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_REMAINDER:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_AND:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_OR:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_XOR:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_AND:
                case AST_EXPR_BINARY_OP_ASSIGNMENT_OR:
                    assignment = true;
                    op = FLOW_GRAPH_EXPR_BINARY_OP_PLUS + (expr->binary.op - AST_EXPR_BINARY_OP_ASSIGNMENT_PLUS);
                    break;
            }

            struct flow_graph_expr * const result = flow_graph_expr_new_binary(
                    expr->position,
                    op,
                    analyze_expr(context, subroutine, expr->binary.lhs, errors),
                    analyze_expr(context, subroutine, expr->binary.rhs, errors)
            );

            if (assignment) {
                return flow_graph_expr_new_binary(
                        expr->position,
                        FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT,
                        check_assignable(subroutine, analyze_expr(context, subroutine, expr->binary.lhs, errors), errors),
                        result
                );
            }

            return result;
        }

        case AST_EXPR_TYPE_UNARY:
            switch (expr->unary.op) {
                case AST_EXPR_UNARY_OP_MINUS:
                case AST_EXPR_UNARY_OP_BITWISE_NOT:
                case AST_EXPR_UNARY_OP_NOT:
                    return flow_graph_expr_new_unary(
                            expr->position,
                            (enum flow_graph_expr_unary_op) expr->unary.op,
                            analyze_expr(context, subroutine, expr->unary.expr, errors)
                    );

                case AST_EXPR_UNARY_OP_INC:
                    return flow_graph_expr_new_binary(
                            expr->position,
                            FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT,
                            analyze_expr(context, subroutine, expr->unary.expr, errors),
                            flow_graph_expr_new_binary(
                                    expr->position,
                                    FLOW_GRAPH_EXPR_BINARY_OP_PLUS,
                                    analyze_expr(context, subroutine, expr->unary.expr, errors),
                                    flow_graph_expr_new_literal(
                                            expr->position,
                                            flow_graph_literal_new_int(expr->position, 1)
                                    )
                            )
                    );

                case AST_EXPR_UNARY_OP_DEC:
                    return flow_graph_expr_new_binary(
                            expr->position,
                            FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT,
                            analyze_expr(context, subroutine, expr->unary.expr, errors),
                            flow_graph_expr_new_binary(
                                    expr->position,
                                    FLOW_GRAPH_EXPR_BINARY_OP_MINUS,
                                    analyze_expr(context, subroutine, expr->unary.expr, errors),
                                    flow_graph_expr_new_literal(
                                            expr->position,
                                            flow_graph_literal_new_int(expr->position, 1)
                                    )
                            )
                    );
            }

        case AST_EXPR_TYPE_BRACES:
            return analyze_expr(context, subroutine, expr->braces.expr, errors);

        case AST_EXPR_TYPE_CALL: {
            const struct ast_expr * const func_expr = expr->call.function;

            if (func_expr->_type != AST_EXPR_TYPE_PLACE) {
                raise_error("complex callee is not allowed", subroutine->filename, func_expr->position, errors);
                return default_expr(expr);
            }

            const struct ast_analyze_reference * ref = lookup_context(
                    subroutine->filename,
                    func_expr->position,
                    context,
                    func_expr->place.id,
                    errors
            );

            if (!ref) {
                return default_expr(expr);
            }

            if (ref->_type != AST_ANALYZE_REFERENCE_TYPE_GLOBAL) {
                raise_error("must be a subroutine", subroutine->filename, func_expr->position, errors);
                return default_expr(expr);
            }

            struct flow_graph_expr * result = flow_graph_expr_new_call(expr->position, ref->global.subroutine);

            for (size_t i = 0; i < expr->call.arguments.size; ++i) {
                flow_graph_expr_list_append(
                        &result->call.args,
                        analyze_expr(context, subroutine, expr->call.arguments.values[i], errors)
                );
            }

            return result;
        }

        case AST_EXPR_TYPE_INDEXER: {
            struct flow_graph_expr * const result = flow_graph_expr_new_indexer(
                    expr->position,
                    analyze_expr(context, subroutine, expr->indexer.value, errors)
            );

            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                flow_graph_expr_list_append(
                        &result->indexer.indices,
                        analyze_expr(context, subroutine, expr->indexer.indices.values[i], errors)
                );
            }

            return result;
        }

        case AST_EXPR_TYPE_PLACE: {
            const struct ast_analyze_reference * ref = lookup_context(
                    subroutine->filename,
                    expr->position,
                    context,
                    expr->place.id,
                    errors
            );

            if (!ref) {
                return default_expr(expr);
            }

            if (ref->_type != AST_ANALYZE_REFERENCE_TYPE_LOCAL) {
                raise_error("must be a local variable", subroutine->filename, expr->position, errors);
                return default_expr(expr);
            }

            return flow_graph_expr_new_local(expr->position, ref->local.local);
        }

        case AST_EXPR_TYPE_LITERAL:
            return flow_graph_expr_new_literal(
                    expr->position,
                    analyze_literal(subroutine->filename, expr->literal.value, errors)
            );
    }

    unreachable();
}

static struct flow_graph_node * append_node(
        struct flow_graph_node * node,
        struct flow_graph_subroutine * subroutine
) {
    flow_graph_node_list_append(&subroutine->nodes, node);
    return node;
}

static struct flow_graph_node * nop(struct position position) {
    return flow_graph_node_new_expr(position, NULL);
}

static void analyze_stmt(
        struct ast_analyze_context * context,
        struct flow_graph_subroutine * subroutine,
        struct flow_graph_node * break_node,
        struct flow_graph_node ** prev_node_next,
        const struct ast_stmt * stmt,
        struct ast_analyze_error_list * errors
) {
    if (!stmt) {
        return;
    }

    switch (stmt->_type) {
        case AST_STMT_TYPE_VAR:
            for (size_t i = 0; i < stmt->var.ids.size; ++i) {
                const struct ast_stmt_var_id * const id = &stmt->var.ids.values[i];

                struct flow_graph_local * const local = append_local(flow_graph_local_new(
                        strdup(id->id),
                        ast_type_reference_clone(stmt->var.type),
                        id->position
                ), subroutine);

                if (id->value) {
                    struct flow_graph_expr * const rhs = analyze_expr(context, subroutine, id->value, errors);
                    struct flow_graph_expr * const expr = flow_graph_expr_new_binary(
                            id->position,
                            FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT,
                            flow_graph_expr_new_local(id->position, local),
                            rhs
                    );

                    struct flow_graph_node * const node = append_node(
                            flow_graph_node_new_expr(id->position, expr),
                            subroutine
                    );

                    node->expr.next = *prev_node_next;
                    *prev_node_next = node;
                    prev_node_next = &node->expr.next;
                }

                ast_analyze_reference_list_append(
                        &context->references,
                        ast_analyze_reference_init_local(local->id, local)
                );
            }

            break;

        case AST_STMT_TYPE_IF: {
            struct flow_graph_node * const cond_node = append_node(
                    flow_graph_node_new_cond(
                            stmt->position,
                            analyze_expr(context, subroutine, stmt->_if.condition, errors)
                    ),
                    subroutine
            );

            cond_node->cond.then_next = *prev_node_next;
            cond_node->cond.else_next = *prev_node_next;
            *prev_node_next = cond_node;

            analyze_stmt(context, subroutine, break_node, &cond_node->cond.then_next, stmt->_if.then_branch, errors);
            analyze_stmt(context, subroutine, break_node, &cond_node->cond.else_next, stmt->_if.else_branch, errors);
            break;
        }

        case AST_STMT_TYPE_BLOCK: {
            struct ast_analyze_context inner_context = ast_analyze_context_init_cons(context);

            for (size_t i = 0; i < stmt->block.stmts.size; ++i) {
                struct ast_stmt * const inner_stmt = stmt->block.stmts.values[i];

                struct flow_graph_node * nop_node = append_node(nop(inner_stmt->position), subroutine);
                nop_node->expr.next = *prev_node_next;
                *prev_node_next = nop_node;

                analyze_stmt(&inner_context, subroutine, break_node, prev_node_next, inner_stmt, errors);

                prev_node_next = &nop_node->expr.next;
            }

            ast_analyze_context_fini(&inner_context);
            break;
        }

        case AST_STMT_TYPE_WHILE: {
            struct flow_graph_node * const next_node = *prev_node_next;

            struct flow_graph_node * const cond_node = append_node(
                    flow_graph_node_new_cond(
                            stmt->position,
                            analyze_expr(context, subroutine, stmt->_while.condition, errors)
                    ),
                    subroutine
            );

            cond_node->cond.then_next = cond_node;
            cond_node->cond.else_next = next_node;
            *prev_node_next = cond_node;

            analyze_stmt(context, subroutine, next_node, &cond_node->cond.then_next, stmt->_while.body, errors);
            break;
        }

        case AST_STMT_TYPE_DO: {
            struct flow_graph_node * const next_node = *prev_node_next;

            struct flow_graph_node * const cond_node = append_node(
                    flow_graph_node_new_cond(
                            stmt->position,
                            analyze_expr(context, subroutine, stmt->_do.condition, errors)
                    ),
                    subroutine
            );

            cond_node->cond.else_next = next_node;
            *prev_node_next = cond_node;

            analyze_stmt(context, subroutine, next_node, prev_node_next, stmt->_do.body, errors);
            cond_node->cond.then_next = *prev_node_next;
            break;
        }

        case AST_STMT_TYPE_BREAK:
            *prev_node_next = break_node;
            break;

        case AST_STMT_TYPE_EXPR: {
            struct flow_graph_node * const node = append_node(
                    flow_graph_node_new_expr(
                            stmt->position,
                            analyze_expr(context, subroutine, stmt->expr.expr, errors)
                    ),
                    subroutine
            );

            node->expr.next = *prev_node_next;
            *prev_node_next = node;
            break;
        }
    }
}

static void remove_nops(struct flow_graph_node ** node_next) {
    struct flow_graph_node * prev = 0;

    while (*node_next) {
        if ((*node_next)->_type != FLOW_GRAPH_NODE_TYPE_EXPR || (*node_next)->expr.expr) {
            break;
        }

        if (*node_next == prev) {
            break;
        }

        prev = *node_next;
        *node_next = (*node_next)->expr.next;
    }
}

static void assign_indexes(struct flow_graph_node * node, size_t * index) {
    if (!node) {
        return;
    }

    if (node->index) {
        return;
    }

    node->index = ++(*index);

    switch (node->_type) {
        case FLOW_GRAPH_NODE_TYPE_EXPR:
            assign_indexes(node->expr.next, index);
            break;

        case FLOW_GRAPH_NODE_TYPE_COND:
            assign_indexes(node->cond.then_next, index);
            assign_indexes(node->cond.else_next, index);
            break;
    }
}

static struct ast_type_reference * default_type(struct position position) {
    return ast_type_reference_new_builtin(position, AST_TYPE_REFERENCE_BUILTIN_TYPE_INT);
}

static struct ast_type_reference * get_literal_type(const struct flow_graph_literal * literal) {
    if (!literal) {
        return NULL;
    }

    switch (literal->_type) {
        case FLOW_GRAPH_LITERAL_TYPE_BOOL:
            return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL);

        case FLOW_GRAPH_LITERAL_TYPE_STR:
            return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_STRING);

        case FLOW_GRAPH_LITERAL_TYPE_CHAR:
            return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_CHAR);

        case FLOW_GRAPH_LITERAL_TYPE_INT:
            if (literal->_int.value <= UINT8_MAX) {
                return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_BYTE);
            }

            if (literal->_int.value <= INT32_MAX) {
                return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_INT);
            }

            if (literal->_int.value <= UINT32_MAX) {
                return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT);
            }

            if (literal->_int.value <= INT64_MAX) {
                return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG);
            }

            return ast_type_reference_new_builtin(literal->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG);
    }

    unreachable();
}

static void fill_types(const char * filename, struct flow_graph_expr * expr, struct ast_analyze_error_list * errors) {
    if (!expr) {
        return;
    }

    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_BINARY:
            fill_types(filename, expr->binary.lhs, errors);
            fill_types(filename, expr->binary.rhs, errors);

            const bool boo = ast_type_reference_is_bool(expr->binary.lhs->type)
                    && ast_type_reference_is_bool(expr->binary.rhs->type);
            const bool num = ast_type_reference_is_numeric(expr->binary.lhs->type)
                    && ast_type_reference_is_numeric(expr->binary.rhs->type);

            switch (expr->binary.op) {
                case FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT:
                    if (!ast_type_reference_is_subtype(expr->binary.rhs->type, expr->binary.lhs->type)) {
                        raise_error("value type must be subtype of variable type", filename, expr->position, errors);
                    }

                    expr->type = ast_type_reference_clone(expr->binary.lhs->type);
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_PLUS:
                case FLOW_GRAPH_EXPR_BINARY_OP_MINUS:
                case FLOW_GRAPH_EXPR_BINARY_OP_MULTIPLY:
                case FLOW_GRAPH_EXPR_BINARY_OP_DIVIDE:
                case FLOW_GRAPH_EXPR_BINARY_OP_REMAINDER:
                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_AND:
                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_OR:
                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_XOR:
                case FLOW_GRAPH_EXPR_BINARY_OP_LEFT_BITSHIFT:
                case FLOW_GRAPH_EXPR_BINARY_OP_RIGHT_BITSHIFT:
                    if (!num) {
                        raise_error("types of operands must be numeric", filename, expr->position, errors);
                        expr->type = default_type(expr->position);
                    } else {
                        expr->type = ast_type_reference_clone(expr->binary.lhs->type);
                    }

                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_AND:
                case FLOW_GRAPH_EXPR_BINARY_OP_OR:
                    if (!boo) {
                        raise_error("types of operands must be bool", filename, expr->position, errors);
                    }

                    expr->type = ast_type_reference_new_builtin(expr->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL);
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_EQ:
                case FLOW_GRAPH_EXPR_BINARY_OP_NE:
                case FLOW_GRAPH_EXPR_BINARY_OP_LT:
                case FLOW_GRAPH_EXPR_BINARY_OP_LE:
                case FLOW_GRAPH_EXPR_BINARY_OP_GT:
                case FLOW_GRAPH_EXPR_BINARY_OP_GE:
                    if (!num) {
                        raise_error("comparison of complex types is not supported", filename, expr->position, errors);
                    }

                    expr->type = ast_type_reference_new_builtin(expr->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL);
                    break;
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_UNARY: {
            struct flow_graph_expr * const value = expr->unary.value;
            fill_types(filename, value, errors);

            switch (expr->unary.op) {
                case FLOW_GRAPH_EXPR_UNARY_OP_NOT:
                    if (ast_type_reference_is_bool(value->type)) {
                        expr->type = ast_type_reference_new_builtin(expr->position, AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL);
                    } else {
                        expr->type = default_type(expr->position);
                        raise_error("value type must be bool", filename, value->position, errors);
                    }

                    break;

                case FLOW_GRAPH_EXPR_UNARY_OP_MINUS:
                case FLOW_GRAPH_EXPR_UNARY_OP_BITWISE_NOT:
                    if (ast_type_reference_is_numeric(value->type)) {
                        expr->type = ast_type_reference_clone(value->type);
                    } else {
                        expr->type = default_type(expr->position);
                        raise_error("value type must be numeric", filename, value->position, errors);
                    }
            }

            break;
        }

        case FLOW_GRAPH_EXPR_TYPE_CALL:
            if (expr->call.subroutine->args_num != expr->call.args.size) {
                raise_error("incorrect number of arguments", filename, expr->position, errors);
            } else {
                for (size_t i = 0; i < expr->call.args.size; ++i) {
                    struct flow_graph_expr * const arg = expr->call.args.values[i];

                    fill_types(filename, arg, errors);

                    if (!ast_type_reference_is_subtype(arg->type, expr->call.subroutine->locals.values[i]->type)) {
                        raise_error("incorrect type of argument", filename, arg->position, errors);
                    }
                }
            }

            expr->type = ast_type_reference_clone(expr->call.subroutine->return_type);
            break;

        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
            fill_types(filename, expr->indexer.value, errors);

            if (expr->indexer.value->type->_type != AST_TYPE_REFERENCE_TYPE_ARRAY) {
                raise_error("cannot index non-array type", filename, expr->position, errors);

                expr->type = default_type(expr->position);
            } else {
                expr->type = ast_type_reference_clone(expr->indexer.value->type->array.type);

                if (expr->indexer.value->type->array.axes != expr->indexer.indices.size) {
                    raise_error("incorrect number of indices", filename, expr->position, errors);
                }
            }

            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                struct flow_graph_expr * const index = expr->indexer.indices.values[i];

                fill_types(filename, index, errors);

                if (!ast_type_reference_is_numeric(index->type)) {
                    raise_error("index type must be numeric", filename, index->position, errors);
                }
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_LOCAL:
            assert(expr->local.local->type);
            expr->type = ast_type_reference_clone(expr->local.local->type);
            break;

        case FLOW_GRAPH_EXPR_TYPE_LITERAL:
            expr->type = get_literal_type(expr->literal.literal);
            break;
    }
}

static void check_return_types(
        const struct flow_graph_subroutine * subroutine,
        const struct flow_graph_node * node,
        struct ast_analyze_error_list * errors
) {
    if (!node) {
        return;
    }

    switch (node->_type) {
        case FLOW_GRAPH_NODE_TYPE_EXPR: {
            if (node->expr.next) {
                break;
            }


            bool ok = true;
            if (node->expr.expr) {
                ok = ast_type_reference_is_subtype(node->expr.expr->type, subroutine->return_type);
            } else {
                ok = ast_type_reference_is_numeric(subroutine->return_type);
            }

            if (!ok) {
                raise_error(
                        "return value type is not a subtype of return type",
                        subroutine->filename,
                        node->position,
                        errors
                );
            }

            break;
        }

        case FLOW_GRAPH_NODE_TYPE_COND:
            if (node->cond.then_next && node->cond.else_next) {
                break;
            }

            if (!ast_type_reference_is_numeric(subroutine->return_type)) {
                raise_error(
                        "return value expected for non-numeric return types",
                        subroutine->filename,
                        node->position,
                        errors
                );
            }

            break;
    }
}

static struct flow_graph_subroutine * lookup_subroutine(
        const struct flow_graph_subroutine_list * subroutines,
        const char * id
) {
    for (size_t i = 0; i < subroutines->size; ++i) {
        if (strcmp(subroutines->values[i]->id, id) == 0) {
            return subroutines->values[i];
        }
    }

    return NULL;
}

static int node_cmp(const void * lhsp, const void * rhsp) {
    const struct flow_graph_node * const lhs = *(struct flow_graph_node **) lhsp;
    const struct flow_graph_node * const rhs = *(struct flow_graph_node **) rhsp;

    if (lhs->index == 0) {
        return (int) (rhs->index - lhs->index);
    }

    if (rhs->index == 0) {
        return -(int) (lhs->index - rhs->index);
    }

    if (lhs->index < rhs->index) {
        return -1;
    }

    return (int) (lhs->index - rhs->index);
}

void ast_analyze(
        const struct ast_analyze_source_list * sources,
        struct flow_graph_subroutine_list * subroutines,
        struct ast_analyze_error_list * errors
) {
    *subroutines = flow_graph_subroutine_list_init();
    *errors = ast_analyze_error_list_init();

    struct ast_analyze_context global_context = create_global_context(sources, subroutines, errors);

    if (errors->size > 0) {
        goto end;
    }

    // назначаем всем аргументам и результатам без указания типов тип по-умолчанию (int)

    for (size_t i = 0; i < subroutines->size; ++i) {
        struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        if (!subroutine->return_type) {
            subroutine->return_type = default_type(subroutine->position);
        }

        for (size_t j = 0; j < subroutine->locals.size; ++j) {
            struct flow_graph_local * const local = subroutine->locals.values[j];

            if (!local->type) {
                local->type = default_type(local->position);
            }
        }
    }

    // формируем граф потока управления для каждой подпрограммы

    for (size_t i = 0; errors->size == 0 && i < sources->size; ++i) {
        const struct ast_source * const source = sources->values[i].source;

        for (size_t j = 0; errors->size == 0 && j < source->items.size; ++j) {
            const struct ast_source_item * const item = source->items.values[j];

            switch (item->_type) {
                case AST_SOURCE_ITEM_TYPE_FUNC_DECL: {
                    if (!item->func_decl.body) {
                        continue;
                    }

                    struct ast_stmt * const body = item->func_decl.body;
                    assert(body->_type == AST_STMT_TYPE_BLOCK);

                    struct flow_graph_subroutine *subroutine =
                            lookup_subroutine(subroutines, item->func_decl.signature->id);

                    assert(subroutine);

                    struct flow_graph_node * first_node = append_node(nop(body->position), subroutine);

                    struct ast_analyze_context context = ast_analyze_context_init_cons(&global_context);

                    for (size_t k = 0; k < item->func_decl.signature->args.size; ++k) {
                        struct flow_graph_local * arg = subroutine->locals.values[k];

                        ast_analyze_reference_list_append(
                                &context.references,
                                ast_analyze_reference_init_local(arg->id, arg)
                        );
                    }

                    analyze_stmt(
                            &context,
                            subroutine,
                            NULL,
                            &first_node->expr.next,
                            body,
                            errors
                    );

                    ast_analyze_context_fini(&context);
                    break;
                }
            }
        }
    }

    // удаляем лишние нопы

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        for (size_t j = subroutine->nodes.size; j > 0; --j) {
            struct flow_graph_node * const node = subroutine->nodes.values[j - 1];

            switch (node->_type) {
                case FLOW_GRAPH_NODE_TYPE_EXPR:
                    remove_nops(&node->expr.next);
                    break;

                case FLOW_GRAPH_NODE_TYPE_COND:
                    remove_nops(&node->cond.then_next);
                    remove_nops(&node->cond.else_next);
                    break;
            }
        }
    }

    // проставляем номера вершинам в порядке вызова

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        if (subroutine->nodes.size == 0) {
            continue;
        }

        size_t index = 0;
        assign_indexes(subroutine->nodes.values[0], &index);
    }

    if (errors->size > 0) {
        goto end;
    }

    // удаляем недостижимые вершины

    for (size_t i = 0; i < subroutines->size; ++i) {
        struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        if (!subroutine->nodes.size) {
            continue;
        }

        qsort(subroutine->nodes.values, subroutine->nodes.size, sizeof(struct flow_graph_node *), node_cmp);

        while (subroutine->nodes.size) {
            const size_t j = subroutine->nodes.size - 1;

            if (subroutine->nodes.values[j]->index) {
                break;
            }

            flow_graph_node_delete(subroutine->nodes.values[j]);
            subroutine->nodes.values[j] = NULL;

            --subroutine->nodes.size;
        }
    }

    // заполнение выражений типами, проверка типов, проверка количества аргументов в вызовах функций и индексации

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        for (size_t j = 0; j < subroutine->nodes.size; ++j) {
            const struct flow_graph_node * const node = subroutine->nodes.values[j];

            switch (node->_type) {
                case FLOW_GRAPH_NODE_TYPE_EXPR:
                    fill_types(subroutine->filename, node->expr.expr, errors);
                    break;

                case FLOW_GRAPH_NODE_TYPE_COND:
                    fill_types(subroutine->filename, node->cond.cond, errors);

                    if (!ast_type_reference_is_bool(node->cond.cond->type)) {
                        raise_error(
                                "condition must be of bool type",
                                subroutine->filename,
                                node->cond.cond->position,
                                errors
                        );
                    }

                    break;
            }
        }
    }

    // проверяем возвращаемые значения функций

    for (size_t i = 0; i < subroutines->size; ++i) {
        const struct flow_graph_subroutine * const subroutine = subroutines->values[i];

        for (size_t j = 0; j < subroutine->nodes.size; ++j) {
            check_return_types(subroutine, subroutine->nodes.values[j], errors);
        }
    }

end:
    ast_analyze_context_fini(&global_context);
}
