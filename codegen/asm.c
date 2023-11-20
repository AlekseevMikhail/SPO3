#include "asm.h"

#include <string.h>

#include "utils/mallocs.h"
#include "utils/unreachable.h"


static const char * const OPCODE_NAME[] = {
        [CODEGEN_ASM_OP_OPCODE_CONST] = "const",
        [CODEGEN_ASM_OP_OPCODE_LOAD] = "load",
        [CODEGEN_ASM_OP_OPCODE_STORE] = "store",
        [CODEGEN_ASM_OP_OPCODE_GET] = "get",
        [CODEGEN_ASM_OP_OPCODE_SET] = "set",
        [CODEGEN_ASM_OP_OPCODE_ZEXT] = "zext",
        [CODEGEN_ASM_OP_OPCODE_SEXT] = "sext",
        [CODEGEN_ASM_OP_OPCODE_TRUNC] = "trunc",
        [CODEGEN_ASM_OP_OPCODE_ADD] = "add",
        [CODEGEN_ASM_OP_OPCODE_SUB] = "sub",
        [CODEGEN_ASM_OP_OPCODE_MUL] = "mul",
        [CODEGEN_ASM_OP_OPCODE_DIV] = "div",
        [CODEGEN_ASM_OP_OPCODE_REM] = "rem",
        [CODEGEN_ASM_OP_OPCODE_AND] = "andb",
        [CODEGEN_ASM_OP_OPCODE_OR] = "orb",
        [CODEGEN_ASM_OP_OPCODE_XOR] = "xorb",
        [CODEGEN_ASM_OP_OPCODE_SHL] = "shl",
        [CODEGEN_ASM_OP_OPCODE_SHR] = "shr",
        [CODEGEN_ASM_OP_OPCODE_CMP] = "cmp",
        [CODEGEN_ASM_OP_OPCODE_GOTO] = "goto",
        [CODEGEN_ASM_OP_OPCODE_IFZ] = "ifz",
        [CODEGEN_ASM_OP_OPCODE_CALL] = "call",
        [CODEGEN_ASM_OP_OPCODE_RET] = "ret",
        [CODEGEN_ASM_OP_OPCODE_NOP] = "nop",
        [CODEGEN_ASM_OP_OPCODE_HLT] = "hlt",
        [CODEGEN_ASM_OP_OPCODE_IN] = "in",
        [CODEGEN_ASM_OP_OPCODE_OUT] = "out",
};

static const char * const REG_NAME[] = {
        [CODEGEN_ASM_OP_REG_SP] = "sp",
        [CODEGEN_ASM_OP_REG_FP] = "fp",
};

static const char * const CMP_NAME[] = {
        [CODEGEN_ASM_OP_CMP_EQ] = "eq",
        [CODEGEN_ASM_OP_CMP_NE] = "ne",
        [CODEGEN_ASM_OP_CMP_LT] = "lt",
        [CODEGEN_ASM_OP_CMP_LE] = "le",
        [CODEGEN_ASM_OP_CMP_GT] = "gt",
        [CODEGEN_ASM_OP_CMP_GE] = "ge",
};

struct codegen_asm codegen_asm_init_comment(char * comment) {
    return (struct codegen_asm) {
        ._type = CODEGEN_ASM_TYPE_COMMENT,
        .comment = comment,
    };
}

struct codegen_asm codegen_asm_init_label(char * label) {
    return (struct codegen_asm) {
        ._type = CODEGEN_ASM_TYPE_LABEL,
        .label = label,
    };
}

struct codegen_asm codegen_asm_init_op(enum codegen_asm_op_opcode opcode) {
    return (struct codegen_asm) {
        ._type = CODEGEN_ASM_TYPE_OP,
        .op = (struct codegen_asm_op) {
            .opcode = opcode,
        },
    };
}

struct codegen_asm codegen_asm_init_data(size_t size, unsigned char * data) {
    return (struct codegen_asm) {
        ._type = CODEGEN_ASM_TYPE_DATA,
        .data = (struct codegen_asm_data) {
            .size = size,
            .data = data,
        },
    };
}

struct codegen_asm codegen_asm_init_label_data(char * label) {
    return (struct codegen_asm) {
        ._type = CODEGEN_ASM_TYPE_LABEL_DATA,
        .label_data = label,
    };
}

void codegen_asm_print(struct codegen_asm value, FILE * file) {
    switch (value._type) {
        case CODEGEN_ASM_TYPE_COMMENT:
            fprintf(file, "; %s", value.comment);
            break;

        case CODEGEN_ASM_TYPE_LABEL:
            fprintf(file, "%s:", value.label);
            break;

        case CODEGEN_ASM_TYPE_OP:
            switch (value.op.opcode) {
                case CODEGEN_ASM_OP_OPCODE_CONST:
                case CODEGEN_ASM_OP_OPCODE_LOAD:
                case CODEGEN_ASM_OP_OPCODE_STORE:
                    fprintf(file, "%s %d", OPCODE_NAME[value.op.opcode], value.op.imm8);
                    break;

                case CODEGEN_ASM_OP_OPCODE_GET:
                case CODEGEN_ASM_OP_OPCODE_SET:
                    fprintf(file, "%s %s", OPCODE_NAME[value.op.opcode], REG_NAME[value.op.reg]);
                    break;

                case CODEGEN_ASM_OP_OPCODE_ZEXT:
                case CODEGEN_ASM_OP_OPCODE_SEXT:
                case CODEGEN_ASM_OP_OPCODE_TRUNC:
                    fprintf(file, "%s %d", OPCODE_NAME[value.op.opcode], value.op.imm2);
                    break;

                case CODEGEN_ASM_OP_OPCODE_ADD:
                case CODEGEN_ASM_OP_OPCODE_SUB:
                case CODEGEN_ASM_OP_OPCODE_MUL:
                case CODEGEN_ASM_OP_OPCODE_DIV:
                case CODEGEN_ASM_OP_OPCODE_REM:
                case CODEGEN_ASM_OP_OPCODE_AND:
                case CODEGEN_ASM_OP_OPCODE_OR:
                case CODEGEN_ASM_OP_OPCODE_XOR:
                case CODEGEN_ASM_OP_OPCODE_SHL:
                case CODEGEN_ASM_OP_OPCODE_SHR:
                case CODEGEN_ASM_OP_OPCODE_RET:
                case CODEGEN_ASM_OP_OPCODE_IN:
                case CODEGEN_ASM_OP_OPCODE_OUT:
                case CODEGEN_ASM_OP_OPCODE_NOP:
                case CODEGEN_ASM_OP_OPCODE_HLT:
                    fprintf(file, "%s", OPCODE_NAME[value.op.opcode]);
                    break;

                case CODEGEN_ASM_OP_OPCODE_CMP:
                    fprintf(file, "%s %s", OPCODE_NAME[value.op.opcode], CMP_NAME[value.op.cmp]);
                    break;

                case CODEGEN_ASM_OP_OPCODE_GOTO:
                case CODEGEN_ASM_OP_OPCODE_IFZ:
                case CODEGEN_ASM_OP_OPCODE_CALL:
                    fprintf(file, "%s %s", OPCODE_NAME[value.op.opcode], value.op.label);
                    break;
            }

            break;

        case CODEGEN_ASM_TYPE_DATA:
            if (value.data.size > 0) {
                fprintf(file, "db 0x%x", value.data.data[0]);

                for (size_t i = 1; i < value.data.size; ++i) {
                    fprintf(file, ", 0x%x", value.data.data[i]);
                }
            }

            break;

        case CODEGEN_ASM_TYPE_LABEL_DATA:
            fprintf(file, "dd %s", value.label_data);
            break;
    }
}

struct codegen_asm codegen_asm_clone(struct codegen_asm value) {
    switch (value._type) {
        case CODEGEN_ASM_TYPE_COMMENT:
            return codegen_asm_init_comment(strdup(value.comment));

        case CODEGEN_ASM_TYPE_LABEL:
            return codegen_asm_init_label(strdup(value.label));

        case CODEGEN_ASM_TYPE_OP: {
            struct codegen_asm result = codegen_asm_init_op(value.op.opcode);

            switch (value.op.opcode) {
                case CODEGEN_ASM_OP_OPCODE_CONST:
                case CODEGEN_ASM_OP_OPCODE_LOAD:
                case CODEGEN_ASM_OP_OPCODE_STORE:
                    result.op.imm8 = value.op.imm8;
                    break;

                case CODEGEN_ASM_OP_OPCODE_GET:
                case CODEGEN_ASM_OP_OPCODE_SET:
                    result.op.reg = value.op.reg;
                    break;

                case CODEGEN_ASM_OP_OPCODE_ZEXT:
                case CODEGEN_ASM_OP_OPCODE_SEXT:
                case CODEGEN_ASM_OP_OPCODE_TRUNC:
                    result.op.imm2 = value.op.imm2;
                    break;

                case CODEGEN_ASM_OP_OPCODE_ADD:
                case CODEGEN_ASM_OP_OPCODE_SUB:
                case CODEGEN_ASM_OP_OPCODE_MUL:
                case CODEGEN_ASM_OP_OPCODE_DIV:
                case CODEGEN_ASM_OP_OPCODE_REM:
                case CODEGEN_ASM_OP_OPCODE_AND:
                case CODEGEN_ASM_OP_OPCODE_OR:
                case CODEGEN_ASM_OP_OPCODE_XOR:
                case CODEGEN_ASM_OP_OPCODE_SHL:
                case CODEGEN_ASM_OP_OPCODE_SHR:
                case CODEGEN_ASM_OP_OPCODE_RET:
                case CODEGEN_ASM_OP_OPCODE_NOP:
                case CODEGEN_ASM_OP_OPCODE_HLT:
                case CODEGEN_ASM_OP_OPCODE_IN:
                case CODEGEN_ASM_OP_OPCODE_OUT:
                    break;

                case CODEGEN_ASM_OP_OPCODE_CMP:
                    result.op.cmp = value.op.cmp;
                    break;

                case CODEGEN_ASM_OP_OPCODE_GOTO:
                case CODEGEN_ASM_OP_OPCODE_IFZ:
                case CODEGEN_ASM_OP_OPCODE_CALL:
                    result.op.label = strdup(value.op.label);
                    break;
            }

            return result;
        }

        case CODEGEN_ASM_TYPE_DATA: {
            void * const d = mallocs(value.data.size);
            memcpy(d, value.data.data, value.data.size);
            return codegen_asm_init_data(value.data.size, d);
        }

        case CODEGEN_ASM_TYPE_LABEL_DATA:
            return codegen_asm_init_label_data(strdup(value.label_data));
    }

    unreachable();
}

void codegen_asm_fini(struct codegen_asm * value) {
    switch (value->_type) {
        case CODEGEN_ASM_TYPE_COMMENT:
            free(value->comment);
            break;

        case CODEGEN_ASM_TYPE_LABEL:
            free(value->label);
            break;

        case CODEGEN_ASM_TYPE_OP:
            switch (value->op.opcode) {
                case CODEGEN_ASM_OP_OPCODE_GOTO:
                case CODEGEN_ASM_OP_OPCODE_IFZ:
                case CODEGEN_ASM_OP_OPCODE_CALL:
                    free(value->op.label);
                    break;

                default:
                    break;
            }

            break;

        case CODEGEN_ASM_TYPE_DATA:
            free(value->data.data);
            break;

        case CODEGEN_ASM_TYPE_LABEL_DATA:
            free(value->label_data);
            break;
    }
}

struct codegen_asm_list codegen_asm_list_init(void) {
    return (struct codegen_asm_list) {
            .size = 0,
            .capacity = 1,
            .values = mallocs(sizeof(struct codegen_asm)),
    };
}

void codegen_asm_list_append(struct codegen_asm_list * list, struct codegen_asm value) {
    if (list->size >= list->capacity) {
        const size_t new_capacity = list->capacity * 2;
        struct codegen_asm * const new_values =
                reallocs(list->values, sizeof(struct codegen_asm) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    list->values[list->size] = value;
    ++list->size;
}

void codegen_asm_list_concat(struct codegen_asm_list * list, struct codegen_asm_list * append) {
    const size_t new_size = list->size + append->size;

    if (new_size > list->capacity) {
        size_t new_capacity = list->capacity;

        while (new_capacity < new_size) {
            new_capacity *= 2;
        }

        struct codegen_asm * const new_values =
                reallocs(list->values, sizeof(struct codegen_asm) * new_capacity);

        list->values = new_values;
        list->capacity = new_capacity;
    }

    for (size_t i = 0; i < append->size; ++i) {
        list->values[list->size++] = append->values[i];
    }

    free(append->values);
    *append = (struct codegen_asm_list) { 0 };
}

struct codegen_asm_list codegen_asm_list_clone(struct codegen_asm_list list) {
    struct codegen_asm_list result = {
            .size = list.size,
            .capacity = list.capacity,
            .values = mallocs(sizeof(struct codegen_asm) * list.capacity),
    };

    for (size_t i = 0; i < list.size; ++i) {
        result.values[i] = codegen_asm_clone(list.values[i]);
    }

    return result;
}

void codegen_asm_list_fini(struct codegen_asm_list * list) {
    for (size_t i = 0; i < list->size; ++i) {
        codegen_asm_fini(&list->values[i]);
    }

    free(list->values);
    *list = (struct codegen_asm_list) { 0 };
}

void codegen_asm_list_print(struct codegen_asm_list value, FILE * file) {
    for (size_t i = 0; i < value.size; ++i) {
        switch (value.values[i]._type) {
            case CODEGEN_ASM_TYPE_OP:
            case CODEGEN_ASM_TYPE_DATA:
            case CODEGEN_ASM_TYPE_LABEL_DATA:
                fputc('\t', file);
                break;

            default:
                break;
        }

        codegen_asm_print(value.values[i], file);
        fputc('\n', file);
    }
}
