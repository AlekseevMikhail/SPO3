#pragma once

#include <stdbool.h>

#include "utils/position.h"


enum ast_literal_type {

    AST_LITERAL_TYPE_BOOL = 0,
    AST_LITERAL_TYPE_STR,
    AST_LITERAL_TYPE_CHAR,
    AST_LITERAL_TYPE_HEX,
    AST_LITERAL_TYPE_BITS,
    AST_LITERAL_TYPE_DEC,
};

struct ast_literal_bool {

    bool value;
};

struct ast_literal_str {

    char * value;
};

struct ast_literal_char {

    char value;
};

struct ast_literal_hex {

    char * value;
};

struct ast_literal_bits {

    char * value;
};

struct ast_literal_dec {

    char * value;
};

struct ast_literal {

    enum ast_literal_type _type;
    struct position position;

    union {
        struct ast_literal_bool _bool;
        struct ast_literal_str str;
        struct ast_literal_char _char;
        struct ast_literal_hex hex;
        struct ast_literal_bits bits;
        struct ast_literal_dec dec;
    };
};

struct ast_literal * ast_literal_new_bool(struct position position, bool value);
struct ast_literal * ast_literal_new_str(struct position position, char * value);
struct ast_literal * ast_literal_new_char(struct position position, char value);
struct ast_literal * ast_literal_new_hex(struct position position, char * value);
struct ast_literal * ast_literal_new_bits(struct position position, char * value);
struct ast_literal * ast_literal_new_dec(struct position position, char * value);
void ast_literal_delete(struct ast_literal * value);
