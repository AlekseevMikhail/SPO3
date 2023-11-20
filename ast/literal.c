#include "literal.h"

#include <string.h>

#include "utils/mallocs.h"
#include "utils/unreachable.h"


struct ast_literal * ast_literal_new_bool(struct position position, bool value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_BOOL;
    result->position = position;
    result->_bool.value = value;

    return result;
}

struct ast_literal * ast_literal_new_str(struct position position, char * value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_STR;
    result->position = position;
    result->str.value = value;

    return result;
}

struct ast_literal * ast_literal_new_char(struct position position, char value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_CHAR;
    result->position = position;
    result->_char.value = value;

    return result;
}

struct ast_literal * ast_literal_new_hex(struct position position, char * value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_HEX;
    result->position = position;
    result->hex.value = value;

    return result;
}

struct ast_literal * ast_literal_new_bits(struct position position, char * value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_BITS;
    result->position = position;
    result->bits.value = value;

    return result;
}

struct ast_literal * ast_literal_new_dec(struct position position, char * value) {
    struct ast_literal * result = mallocs(sizeof(struct ast_literal));

    result->_type = AST_LITERAL_TYPE_DEC;
    result->position = position;
    result->dec.value = value;

    return result;
}

void ast_literal_delete(struct ast_literal * value) {
    if (!value) {
        return;
    }

    switch (value->_type) {
        case AST_LITERAL_TYPE_STR:
            free(value->str.value);
            break;

        case AST_LITERAL_TYPE_HEX:
            free(value->hex.value);
            break;

        case AST_LITERAL_TYPE_BITS:
            free(value->bits.value);
            break;

        case AST_LITERAL_TYPE_DEC:
            free(value->dec.value);
            break;

        case AST_LITERAL_TYPE_BOOL:
        case AST_LITERAL_TYPE_CHAR:
            break;
    }

    free(value);
}
