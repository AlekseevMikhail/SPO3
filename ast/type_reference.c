#include "type_reference.h"

#include <string.h>

#include "utils/mallocs.h"
#include "utils/unreachable.h"


struct ast_type_reference * ast_type_reference_new_builtin(
        struct position position,
        enum ast_type_reference_builtin_type type
) {
    struct ast_type_reference * result = mallocs(sizeof(struct ast_type_reference));

    result->_type = AST_TYPE_REFERENCE_TYPE_BUILTIN;
    result->position = position;
    result->builtin = (struct ast_type_reference_builtin) {
        .type = type,
    };

    return result;
}

struct ast_type_reference * ast_type_reference_new_custom(struct position position, char * id) {
    struct ast_type_reference * result = mallocs(sizeof(struct ast_type_reference));

    result->_type = AST_TYPE_REFERENCE_TYPE_CUSTOM;
    result->position = position;
    result->custom = (struct ast_type_reference_custom) {
        .id = id,
    };

    return result;
}

struct ast_type_reference * ast_type_reference_new_array(
        struct position position,
        struct ast_type_reference * type,
        size_t axes
) {
    struct ast_type_reference * result = mallocs(sizeof(struct ast_type_reference));

    result->_type = AST_TYPE_REFERENCE_TYPE_ARRAY;
    result->position = position;
    result->array = (struct ast_type_reference_array) {
        .type = type,
        .axes = axes,
    };

    return result;
}

struct ast_type_reference * ast_type_reference_clone(struct ast_type_reference * type) {
    if (!type) {
        return NULL;
    }

    switch (type->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            return ast_type_reference_new_builtin(type->position, type->builtin.type);

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
            return ast_type_reference_new_custom(type->position, strdup(type->custom.id));

        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            return ast_type_reference_new_array(
                    type->position,
                    ast_type_reference_clone(type->array.type),
                    type->array.axes
            );
    }

    unreachable();
}

bool ast_type_reference_equals(const struct ast_type_reference * lhs, const struct ast_type_reference * rhs) {
    if (!lhs || !rhs) {
        return lhs == rhs;
    }

    if (lhs->_type != rhs->_type) {
        return false;
    }

    switch (lhs->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            return lhs->builtin.type == rhs->builtin.type;

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
            return strcmp(lhs->custom.id, rhs->custom.id) == 0;

        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            return lhs->array.axes == rhs->array.axes
                   && ast_type_reference_equals(lhs->array.type, rhs->array.type);
    }

    unreachable();
}

bool ast_type_reference_is_subtype(const struct ast_type_reference * lhs, const struct ast_type_reference * rhs) {
    if (!lhs || !rhs) {
        return false;
    }

    switch (lhs->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            switch (lhs->builtin.type) {
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_BYTE:
                    return ast_type_reference_is_numeric(rhs);

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_INT:
                    return rhs->_type == AST_TYPE_REFERENCE_TYPE_BUILTIN
                        && (rhs->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_INT
                            || rhs->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG);

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT:
                    return rhs->_type == AST_TYPE_REFERENCE_TYPE_BUILTIN
                           && (rhs->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT
                               || rhs->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG
                               || rhs->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG);

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_CHAR:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_STRING:
                    return ast_type_reference_equals(lhs, rhs);
            }

            unreachable();

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            return ast_type_reference_equals(lhs, rhs);
    }

    unreachable();
}

bool ast_type_reference_is_numeric(const struct ast_type_reference * type) {
    if (!type) {
        return false;
    }

    if (type->_type != AST_TYPE_REFERENCE_TYPE_BUILTIN) {
        return false;
    }

    switch (type->builtin.type) {
        case AST_TYPE_REFERENCE_BUILTIN_TYPE_BYTE:
        case AST_TYPE_REFERENCE_BUILTIN_TYPE_INT:
        case AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT:
        case AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG:
        case AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG:
            return true;

        default:
            return false;
    }
}

bool ast_type_reference_is_custom(const struct ast_type_reference * type) {
    if (!type) {
        return false;
    }

    return type->_type == AST_TYPE_REFERENCE_TYPE_CUSTOM;
}

bool ast_type_reference_is_bool(const struct ast_type_reference * type) {
    if (!type) {
        return false;
    }

    return type->_type == AST_TYPE_REFERENCE_TYPE_BUILTIN
        && type->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL;
}

void ast_type_reference_delete(struct ast_type_reference * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            break;

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
            free(value->custom.id);
            break;

        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            ast_type_reference_delete(value->array.type);
            break;
    }

    free(value);
}
