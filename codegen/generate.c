#include "generate.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "utils/mallocs.h"
#include "utils/unreachable.h"


struct space {

    struct codegen_asm_list listing;
    const char * label_prefix;
    size_t label_generator;
};

static const char * const NODE_TYPE_NAME[] = {
        [FLOW_GRAPH_NODE_TYPE_EXPR] = "EXPR",
        [FLOW_GRAPH_NODE_TYPE_COND] = "COND",
};

static const char * const LEAVE_LABEL = ".leave";
static const char * const RETURN_VOID_LABEL = ".return_void";
static const char * const NODE_LABEL_PREFIX = "node";

static const size_t POINTER_SIZE = 4;

static const struct ast_type_reference * internal_int_type = NULL;

const char * const codegen_header =
        "[section ram]\n"
        "\tconst 4\n"
        "\tdb 0, 0xf0, 0xff, 0xff\n"
        "\tset sp\n"
        "\tget sp\n"
        "\tset fp\n"
        "\tcall main\n"
        "\thlt\n";

const char * const codegen_footer =
        "; heap\n"
        "heap_end:\n"
        "\tdd heap\n"
        "heap:\n";

const char * const codegen_builtins =
        "; builtin: char read();\n"
        "read:\n"
        "\tget sp\n"
        "\tin\n"
        "\tstore 1\n"
        "\tret\n"
        "; builtin int write(char c);\n"
        "write:\n"
        "\tout\n"
        "\tret\n"
        "; builtin int[] new_int_array(ulong size);\n"
        "new_int_array:\n"
        "\tget fp\n"
        "\tget sp\n"
        "\tset fp\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tconst 4\n"
        "\tdd heap_end\n"
        "\tload 4\n"
        "\tstore 4\n"
        "\tconst 4\n"
        "\tdd heap_end\n"
        "\tload 4\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tstore 4\n"
        "\tconst 4\n"
        "\tdd heap_end\n"
        "\tconst 4\n"
        "\tdd heap_end\n"
        "\tload 4\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tconst 4\n"
        "\tdb 2, 0, 0, 0\n"
        "\tmul\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tadd\n"
        "\tstore 4\n"
        "\tset fp\n"
        "\tget sp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tset sp\n"
        "\tret\n"
        "; builtin ulong get_int_array_size(int[] array);\n"
        "get_int_array_size:\n"
        "\tload 4\n"
        "\tget sp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tget sp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tstore 4\n"
        "\tget sp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tset sp\n"
        "\tret\n"
        "; builtin int write_str(string str);\n"
        "write_str:\n"
        "\tget sp\n"
        "\tload 4\n"
        "\tload 4\n"
        "\tget fp\n"
        "\tget sp\n"
        "\tset fp\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tstore 4\n"
        "\tgoto .loop_cond\n"
        ".loop:\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tload 1\n"
        "\tout\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tconst 4\n"
        "\tdb 1, 0, 0, 0\n"
        "\tadd\n"
        "\tstore 4\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tconst 4\n"
        "\tdb 1, 0, 0, 0\n"
        "\tsub\n"
        "\tstore 4\n"
        ".loop_cond:\n"
        "\tget fp\n"
        "\tconst 4\n"
        "\tdb 4, 0, 0, 0\n"
        "\tadd\n"
        "\tload 4\n"
        "\tconst 4\n"
        "\tdb 0, 0, 0, 0\n"
        "\tcmp eq\n"
        "\tifz .loop\n"
        "\tset fp\n"
        "\tget sp\n"
        "\tconst 4\n"
        "\tdb 8, 0, 0, 0\n"
        "\tadd\n"
        "\tset sp\n"
        "\tret\n"
        "; builtin byte ord(char c);\n"
        "ord:\n"
        "\tsext 2\n"
        "\ttrunc 1\n"
        "\tret\n"
        "; builtin char chr(byte c);\n"
        "chr:\n"
        "\tgoto ord\n";

static char * generate_label(const char * prefix, size_t index) {
    const size_t size = strlen(prefix) + 30;

    char * result = mallocs(sizeof(char) * size);
    snprintf(result, size, ".%s_%zu", prefix, index);

    return result;
}

static struct space space_init(const char * prefix) {
    return (struct space) {
        .listing = codegen_asm_list_init(),
        .label_prefix = prefix,
        .label_generator = 0,
    };
}

static const char * space_new_label(struct space * space) {
    char * const label = generate_label(space->label_prefix, ++space->label_generator);

    codegen_asm_list_append(&space->listing, codegen_asm_init_label(label));
    return label;
}

static size_t get_type_size(const struct ast_type_reference * type) {
    assert(type);

    switch (type->_type) {
        case AST_TYPE_REFERENCE_TYPE_BUILTIN:
            switch (type->builtin.type) {
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_BYTE:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_CHAR:
                    return 1;

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_INT:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT:
                    return 2;

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG:
                case AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG:
                    return 4;

                case AST_TYPE_REFERENCE_BUILTIN_TYPE_STRING:
                    return POINTER_SIZE;
            }

        case AST_TYPE_REFERENCE_TYPE_CUSTOM:
        case AST_TYPE_REFERENCE_TYPE_ARRAY:
            return POINTER_SIZE;
    }

    unreachable();
}

static void cast_to_type(
        const struct ast_type_reference * from,
        const struct ast_type_reference * to,
        struct codegen_asm_list * code
) {
    if (ast_type_reference_equals(from, to)) {
        return;
    }

    assert(ast_type_reference_is_numeric(from));
    assert(ast_type_reference_is_numeric(to));

    const bool is_signed = from->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_INT
                           || from->builtin.type == AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG;

    const size_t from_size = get_type_size(from);

    if (from_size < 4) {
        if (is_signed) {
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SEXT);
            ins.op.imm2 = from_size;
            codegen_asm_list_append(code, ins);
        } else {
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ZEXT);
            ins.op.imm2 = from_size;
            codegen_asm_list_append(code, ins);
        }
    }

    const size_t to_size = get_type_size(to);
    if (to_size < 4) {
        struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_TRUNC);
        ins.op.imm2 = to_size;
        codegen_asm_list_append(code, ins);
    }
}

static void generate_const_int(uint64_t value, struct codegen_asm_list * code) {
    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CONST);
    ins.op.imm8 = 4;
    codegen_asm_list_append(code, ins);

    // db value
    ins = codegen_asm_init_data(4, mallocs(4));
    *(uint32_t *) ins.data.data = (uint32_t) value;
    codegen_asm_list_append(code, ins);
}

static void generate_literal(
        const struct flow_graph_literal * literal,
        const struct ast_type_reference * type,
        struct codegen_asm_list * code,
        struct space * const_space
) {
    switch (literal->_type) {
        case FLOW_GRAPH_LITERAL_TYPE_BOOL: {
            const uint8_t value = literal->_bool.value ? 0xff : 0;

            // const 1
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CONST);
            ins.op.imm8 = 1;
            codegen_asm_list_append(code, ins);

            // db value
            ins = codegen_asm_init_data(1, mallocs(1));
            *ins.data.data = value;
            codegen_asm_list_append(code, ins);

            // sext 1
            ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SEXT);
            ins.op.imm2 = 1;
            codegen_asm_list_append(code, ins);
            break;
        }

        case FLOW_GRAPH_LITERAL_TYPE_STR: {
            const char * const label = space_new_label(const_space);

            // db strlen(value), value
            const size_t len = strlen(literal->str.value);
            struct codegen_asm ins = codegen_asm_init_data(len + 4, mallocs(len + 4));
            *(uint32_t *) ins.data.data = len;
            memcpy(ins.data.data + 4, literal->str.value, len);
            codegen_asm_list_append(&const_space->listing, ins);

            // const POINTER_SIZE
            ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CONST);
            ins.op.imm8 = POINTER_SIZE;
            codegen_asm_list_append(code, ins);

            // dd label
            codegen_asm_list_append(code, codegen_asm_init_label_data(strdup(label)));
            break;
        }

        case FLOW_GRAPH_LITERAL_TYPE_CHAR: {

            // const 1
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CONST);
            ins.op.imm8 = 1;
            codegen_asm_list_append(code, ins);

            // db value
            ins = codegen_asm_init_data(1, mallocs(1));
            *ins.data.data = literal->_char.value;
            codegen_asm_list_append(code, ins);
            break;
        }

        case FLOW_GRAPH_LITERAL_TYPE_INT:
            // const 4
            // db value
            generate_const_int(literal->_int.value, code);
            cast_to_type(internal_int_type, type, code);
            break;
    }
}

static void generate_expr(
        const struct flow_graph_subroutine * subroutine,
        const struct flow_graph_expr * expr,
        struct codegen_asm_list * code,
        struct space * const_space
);

static struct codegen_asm_list generate_expr_for_op(
        const struct flow_graph_subroutine * subroutine,
        const struct flow_graph_expr * expr,
        struct space * const_space
) {
    struct codegen_asm_list result = codegen_asm_list_init();

    generate_expr(subroutine, expr, &result, const_space);

    if (ast_type_reference_is_numeric(expr->type)) {
        cast_to_type(expr->type, internal_int_type, &result);
    }

    return result;
}

static void generate_expr(
        const struct flow_graph_subroutine * subroutine,
        const struct flow_graph_expr * expr,
        struct codegen_asm_list * code,
        struct space * const_space
) {
    switch (expr->_type) {
        case FLOW_GRAPH_EXPR_TYPE_BINARY: {
            if (expr->binary.op == FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT) {
                struct codegen_asm_list access = codegen_asm_list_init();
                size_t offset = 0, size = 0;

                struct flow_graph_expr * const lhs = expr->binary.lhs;

                switch (lhs->_type) {
                    case FLOW_GRAPH_EXPR_TYPE_INDEXER: {
                        assert(lhs->indexer.indices.size > 0);

                        generate_expr(subroutine, lhs->indexer.value, &access, const_space);

                        {
                            generate_expr(subroutine, lhs->indexer.indices.values[0], &access, const_space);
                            cast_to_type(lhs->indexer.indices.values[0]->type, internal_int_type, &access);

                            size = lhs->indexer.indices.size == 1
                                    ? get_type_size(lhs->type)
                                    : POINTER_SIZE;

                            // const 4
                            // db elem_size
                            generate_const_int(size, &access);

                            // mul
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_MUL));

                            // add
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

                            // const 4
                            // db 4, 0, 0, 0
                            generate_const_int(4, &access);

                            // add
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));
                        }

                        for (size_t i = 1; i < lhs->indexer.indices.size; ++i) {
                            // load elem_size
                            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_LOAD);
                            ins.op.imm8 = size;
                            codegen_asm_list_append(&access, ins);

                            generate_expr(subroutine, lhs->indexer.indices.values[i], &access, const_space);
                            cast_to_type(lhs->indexer.indices.values[i]->type, internal_int_type, &access);

                            size = i == lhs->indexer.indices.size - 1
                                    ? get_type_size(lhs->type)
                                    : POINTER_SIZE;

                            // const 4
                            // db elem_size
                            generate_const_int(size, &access);

                            // mul
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_MUL));

                            // add
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

                            // const 4
                            // db 4, 0, 0, 0
                            generate_const_int(4, &access);

                            // add
                            codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));
                        }

                        break;
                    }

                    case FLOW_GRAPH_EXPR_TYPE_LOCAL: {
                        for (size_t i = 0; i < subroutine->locals.size; ++i) {
                            size = get_type_size(subroutine->locals.values[i]->type);
                            offset += size;

                            if (subroutine->locals.values[i] == lhs->local.local) {
                                break;
                            }
                        }

                        // get FP
                        struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
                        ins.op.reg = CODEGEN_ASM_OP_REG_FP;
                        codegen_asm_list_append(&access, ins);

                        // const 4
                        // db offset
                        generate_const_int(offset, &access);

                        // sub
                        codegen_asm_list_append(&access, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));
                        break;
                    }

                    default:
                        unreachable();
                }

                {
                    struct codegen_asm_list tmp = codegen_asm_list_clone(access);
                    codegen_asm_list_concat(code, &tmp);
                }

                generate_expr(subroutine, expr->binary.rhs, code, const_space);
                cast_to_type(expr->binary.rhs->type, lhs->type, code);

                // store size
                struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_STORE);
                ins.op.imm8 = size;
                codegen_asm_list_append(code, ins);

                codegen_asm_list_concat(code, &access);

                // load size
                ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_LOAD);
                ins.op.imm8 = size;
                codegen_asm_list_append(code, ins);

                return;
            }

            struct codegen_asm_list lhs_code = generate_expr_for_op(subroutine, expr->binary.lhs, const_space);
            struct codegen_asm_list rhs_code = generate_expr_for_op(subroutine, expr->binary.rhs, const_space);

            codegen_asm_list_concat(code, &lhs_code);
            codegen_asm_list_concat(code, &rhs_code);

            switch (expr->binary.op) {
                case FLOW_GRAPH_EXPR_BINARY_OP_ASSIGNMENT:
                    unreachable();

                case FLOW_GRAPH_EXPR_BINARY_OP_PLUS:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_MINUS:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_MULTIPLY:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_MUL));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_DIVIDE:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_DIV));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_REMAINDER:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_REM));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_AND:
                case FLOW_GRAPH_EXPR_BINARY_OP_AND:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_AND));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_OR:
                case FLOW_GRAPH_EXPR_BINARY_OP_OR:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_OR));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_BITWISE_XOR:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_XOR));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_EQ: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_EQ;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_NE: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_NE;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_LT: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_LT;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_LE: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_LE;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_GT: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_GT;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_GE: {
                    struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CMP);
                    ins.op.cmp = CODEGEN_ASM_OP_CMP_GE;
                    codegen_asm_list_append(code, ins);
                    break;
                }

                case FLOW_GRAPH_EXPR_BINARY_OP_LEFT_BITSHIFT:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SHL));
                    break;

                case FLOW_GRAPH_EXPR_BINARY_OP_RIGHT_BITSHIFT:
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SHR));
                    break;
            }

            if (ast_type_reference_is_numeric(expr->type)) {
                cast_to_type(internal_int_type, expr->type, code);
            }

            break;
        }

        case FLOW_GRAPH_EXPR_TYPE_UNARY: {
            struct codegen_asm_list value_code = generate_expr_for_op(subroutine, expr->unary.value, const_space);

            switch (expr->unary.op) {
                case FLOW_GRAPH_EXPR_UNARY_OP_MINUS:
                    // const 4
                    // db 0, 0, 0, 0
                    generate_const_int(0, code);

                    // value_code
                    codegen_asm_list_concat(code, &value_code);

                    // sub
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));
                    break;

                case FLOW_GRAPH_EXPR_UNARY_OP_BITWISE_NOT:
                case FLOW_GRAPH_EXPR_UNARY_OP_NOT:
                    // value_code
                    codegen_asm_list_concat(code, &value_code);

                    // const 4
                    // db 0xff, 0xff, 0xff, 0xff
                    generate_const_int(0xffffffff, code);

                    // xor
                    codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_XOR));
                    break;
            }

            if (ast_type_reference_is_numeric(expr->type)) {
                cast_to_type(internal_int_type, expr->type, code);
            }

            break;
        }

        case FLOW_GRAPH_EXPR_TYPE_CALL: {

            // get SP
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
            ins.op.reg = CODEGEN_ASM_OP_REG_SP;
            codegen_asm_list_append(code, ins);

            // const 4
            // db sizeof(return_type)
            generate_const_int(get_type_size(expr->call.subroutine->return_type), code);

            // sub
            codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));

            // set SP
            ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
            ins.op.reg = CODEGEN_ASM_OP_REG_SP;
            codegen_asm_list_append(code, ins);

            for (size_t i = 0; i < expr->call.args.size; ++i) {
                generate_expr(subroutine, expr->call.args.values[i], code, const_space);
                cast_to_type(expr->call.args.values[i]->type, expr->call.subroutine->locals.values[i]->type, code);
            }

            ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_CALL);
            ins.op.label = strdup(expr->call.subroutine->id);
            codegen_asm_list_append(code, ins);
            break;
        }

        case FLOW_GRAPH_EXPR_TYPE_INDEXER:
            generate_expr(subroutine, expr->indexer.value, code, const_space);

            for (size_t i = 0; i < expr->indexer.indices.size; ++i) {
                generate_expr(subroutine, expr->indexer.indices.values[i], code, const_space);
                cast_to_type(expr->indexer.indices.values[i]->type, internal_int_type, code);

                const size_t elem_size = i == expr->indexer.indices.size - 1
                        ? get_type_size(expr->type)
                        : POINTER_SIZE;

                // const 4
                // db elem_size
                generate_const_int(elem_size, code);

                // mul
                codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_MUL));

                // add
                codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

                // const 4
                // db 4, 0, 0, 0
                generate_const_int(4, code);

                // add
                codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

                // load elem_size
                struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_LOAD);
                ins.op.imm8 = elem_size;
                codegen_asm_list_append(code, ins);
            }

            break;

        case FLOW_GRAPH_EXPR_TYPE_LOCAL: {
            size_t offset = 0, size = 0;

            for (size_t i = 0; i < subroutine->locals.size; ++i) {
                size = get_type_size(subroutine->locals.values[i]->type);
                offset += size;

                if (subroutine->locals.values[i] == expr->local.local) {
                    break;
                }
            }

            // get FP
            struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
            ins.op.reg = CODEGEN_ASM_OP_REG_FP;
            codegen_asm_list_append(code, ins);

            // const 4
            // db offset
            generate_const_int(offset, code);

            // sub
            codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));

            // load size
            ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_LOAD);
            ins.op.imm8 = size;
            codegen_asm_list_append(code, ins);
            break;
        }

        case FLOW_GRAPH_EXPR_TYPE_LITERAL:
            generate_literal(expr->literal.literal, expr->type, code, const_space);
            break;
    }
}

static void generate_node_next(
        const struct flow_graph_subroutine * subroutine,
        enum codegen_asm_op_opcode opcode,
        const struct flow_graph_node * node_next,
        struct ast_type_reference * value_type,
        struct codegen_asm_list * code
) {
    struct codegen_asm ins = codegen_asm_init_op(opcode);

    if (node_next) {
        if (value_type) {
            // get SP
            struct codegen_asm ins1 = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
            ins1.op.reg = CODEGEN_ASM_OP_REG_SP;
            codegen_asm_list_append(code, ins1);

            // const 4
            // db sizeof(value_type)
            generate_const_int(get_type_size(value_type), code);

            // add
            codegen_asm_list_append(code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

            // set SP
            ins1 = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
            ins1.op.reg = CODEGEN_ASM_OP_REG_SP;
            codegen_asm_list_append(code, ins1);
        }

        ins.op.label = generate_label(NODE_LABEL_PREFIX, node_next->index);
    } else {
        if (value_type) {
            cast_to_type(value_type, subroutine->return_type, code);
            ins.op.label = strdup(LEAVE_LABEL);
        } else {
            ins.op.label = strdup(RETURN_VOID_LABEL);
        }
    }

    codegen_asm_list_append(code, ins);
}

static void generate_node(
        const struct flow_graph_subroutine * subroutine,
        const struct flow_graph_node * node,
        struct codegen_asm_list * code,
        struct space * const_space
) {
    {
        char comment[1024];
        snprintf(
                comment,
                1024,
                "%zu: %s at %zu:%zu",
                node->index,
                NODE_TYPE_NAME[node->_type],
                node->position.row,
                node->position.column
        );
        codegen_asm_list_append(code, codegen_asm_init_comment(strdup(comment)));
    }

    codegen_asm_list_append(code, codegen_asm_init_label(generate_label(NODE_LABEL_PREFIX, node->index)));

    switch (node->_type) {
        case FLOW_GRAPH_NODE_TYPE_EXPR:
            if (node->expr.expr) {
                generate_expr(subroutine, node->expr.expr, code, const_space);
            }

            generate_node_next(
                    subroutine,
                    CODEGEN_ASM_OP_OPCODE_GOTO,
                    node->expr.next,
                    node->expr.expr ? node->expr.expr->type : NULL,
                    code
            );

            break;

        case FLOW_GRAPH_NODE_TYPE_COND:
            generate_expr(subroutine, node->cond.cond, code, const_space);
            generate_node_next(subroutine, CODEGEN_ASM_OP_OPCODE_IFZ, node->cond.else_next, NULL, code);
            generate_node_next(subroutine, CODEGEN_ASM_OP_OPCODE_GOTO, node->cond.then_next, NULL, code);
            break;
    }
}

static struct codegen_asm_list generate_subroutine(const struct flow_graph_subroutine * subroutine) {
    struct codegen_asm_list code = codegen_asm_list_init();
    struct space const_space = space_init("const");

    {
        size_t size = strlen(subroutine->filename) + 30;
        char * comment = mallocs(sizeof(char) * size);
        snprintf(comment, 1024, "%s:%zu", subroutine->filename, subroutine->position.row);
        codegen_asm_list_append(&code, codegen_asm_init_comment(comment));
    }

    codegen_asm_list_append(&code, codegen_asm_init_label(strdup(subroutine->id)));
    codegen_asm_list_append(&const_space.listing, codegen_asm_init_comment(strdup("constants")));

    {
        // prologue

        size_t locals_size = 0;
        for (size_t i = subroutine->args_num; i < subroutine->locals.size; ++i) {
            locals_size += get_type_size(subroutine->locals.values[i]->type);
        }

        // get SP
        struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
        ins.op.reg = CODEGEN_ASM_OP_REG_SP;
        codegen_asm_list_append(&code, ins);

        // const 4
        // db locals_size
        generate_const_int(locals_size, &code);

        // sub
        codegen_asm_list_append(&code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SUB));

        // set SP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
        ins.op.reg = CODEGEN_ASM_OP_REG_SP;
        codegen_asm_list_append(&code, ins);

        // get FP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
        ins.op.reg = CODEGEN_ASM_OP_REG_FP;
        codegen_asm_list_append(&code, ins);

        // get SP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
        ins.op.reg = CODEGEN_ASM_OP_REG_SP;
        codegen_asm_list_append(&code, ins);

        locals_size += POINTER_SIZE;
        for (size_t i = 0; i < subroutine->args_num; ++i) {
            locals_size += get_type_size(subroutine->locals.values[i]->type);
        }

        // const 4
        // db locals_size
        generate_const_int(locals_size, &code);

        // add
        codegen_asm_list_append(&code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

        // set FP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
        ins.op.reg = CODEGEN_ASM_OP_REG_FP;
        codegen_asm_list_append(&code, ins);

        // get FP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
        ins.op.reg = CODEGEN_ASM_OP_REG_FP;
        codegen_asm_list_append(&code, ins);
    }

    for (size_t i = 0; i < subroutine->nodes.size; ++i) {
        generate_node(subroutine, subroutine->nodes.values[i], &code, &const_space);
    }

    if (ast_type_reference_is_numeric(subroutine->return_type)) {
        codegen_asm_list_append(&code, codegen_asm_init_label(strdup(RETURN_VOID_LABEL)));

        // return void (zero)

        struct flow_graph_literal * lit = flow_graph_literal_new_int(subroutine->position, 0);
        generate_literal(lit, subroutine->return_type, &code, &const_space);
        flow_graph_literal_delete(lit);
    }

    codegen_asm_list_append(&code, codegen_asm_init_label(strdup(LEAVE_LABEL)));

    {
        // epilogue

        // store sizeof(return_type)
        struct codegen_asm ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_STORE);
        ins.op.imm8 = get_type_size(subroutine->return_type);
        codegen_asm_list_append(&code, ins);

        // set FP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
        ins.op.reg = CODEGEN_ASM_OP_REG_FP;
        codegen_asm_list_append(&code, ins);

        size_t locals_size = 0;
        for (size_t i = 0; i < subroutine->locals.size; ++i) {
            locals_size += get_type_size(subroutine->locals.values[i]->type);
        }

        // get SP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_GET);
        ins.op.reg = CODEGEN_ASM_OP_REG_SP;
        codegen_asm_list_append(&code, ins);

        // const 4
        // db locals_size
        generate_const_int(locals_size, &code);

        // add
        codegen_asm_list_append(&code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_ADD));

        // set SP
        ins = codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_SET);
        ins.op.reg = CODEGEN_ASM_OP_REG_SP;
        codegen_asm_list_append(&code, ins);

        // ret
        codegen_asm_list_append(&code, codegen_asm_init_op(CODEGEN_ASM_OP_OPCODE_RET));
    }

    codegen_asm_list_concat(&code, &const_space.listing);
    return code;
}

struct codegen_asm_list codegen_generate(struct flow_graph_subroutine_list subroutines) {
    if (!internal_int_type) {
        internal_int_type = ast_type_reference_new_builtin(
                position_init(0, 0),
                AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG
        );
    }

    struct codegen_asm_list result = codegen_asm_list_init();

    for (size_t i = 0; i < subroutines.size; ++i) {
        const struct flow_graph_subroutine * subroutine = subroutines.values[i];

        if (!subroutine->defined) {
            continue;
        }

        struct codegen_asm_list subroutine_code = generate_subroutine(subroutine);
        codegen_asm_list_concat(&result, &subroutine_code);
    }

    return result;
}
