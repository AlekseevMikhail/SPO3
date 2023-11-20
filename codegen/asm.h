#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


enum codegen_asm_type {

    CODEGEN_ASM_TYPE_COMMENT = 0,
    CODEGEN_ASM_TYPE_LABEL,
    CODEGEN_ASM_TYPE_OP,
    CODEGEN_ASM_TYPE_DATA,
    CODEGEN_ASM_TYPE_LABEL_DATA,
};

enum codegen_asm_op_opcode {

    CODEGEN_ASM_OP_OPCODE_CONST = 0,
    CODEGEN_ASM_OP_OPCODE_LOAD,
    CODEGEN_ASM_OP_OPCODE_STORE,
    CODEGEN_ASM_OP_OPCODE_GET,
    CODEGEN_ASM_OP_OPCODE_SET,
    CODEGEN_ASM_OP_OPCODE_ZEXT,
    CODEGEN_ASM_OP_OPCODE_SEXT,
    CODEGEN_ASM_OP_OPCODE_TRUNC,
    CODEGEN_ASM_OP_OPCODE_ADD,
    CODEGEN_ASM_OP_OPCODE_SUB,
    CODEGEN_ASM_OP_OPCODE_MUL,
    CODEGEN_ASM_OP_OPCODE_DIV,
    CODEGEN_ASM_OP_OPCODE_REM,
    CODEGEN_ASM_OP_OPCODE_AND,
    CODEGEN_ASM_OP_OPCODE_OR,
    CODEGEN_ASM_OP_OPCODE_XOR,
    CODEGEN_ASM_OP_OPCODE_SHL,
    CODEGEN_ASM_OP_OPCODE_SHR,
    CODEGEN_ASM_OP_OPCODE_CMP,
    CODEGEN_ASM_OP_OPCODE_GOTO,
    CODEGEN_ASM_OP_OPCODE_IFZ,
    CODEGEN_ASM_OP_OPCODE_CALL,
    CODEGEN_ASM_OP_OPCODE_RET,
    CODEGEN_ASM_OP_OPCODE_NOP,
    CODEGEN_ASM_OP_OPCODE_HLT,
    CODEGEN_ASM_OP_OPCODE_IN,
    CODEGEN_ASM_OP_OPCODE_OUT,
};

enum codegen_asm_op_reg {

    CODEGEN_ASM_OP_REG_SP = 0,
    CODEGEN_ASM_OP_REG_FP = 1,
};

enum codegen_asm_op_cmp {

    CODEGEN_ASM_OP_CMP_EQ = 0,
    CODEGEN_ASM_OP_CMP_NE = 1,
    CODEGEN_ASM_OP_CMP_LT = 4,
    CODEGEN_ASM_OP_CMP_LE = 5,
    CODEGEN_ASM_OP_CMP_GT = 6,
    CODEGEN_ASM_OP_CMP_GE = 7,
};

struct codegen_asm_op {

    enum codegen_asm_op_opcode opcode;

    union {
        uint8_t imm8;
        uint8_t imm2;
        char * label;
        enum codegen_asm_op_reg reg;
        enum codegen_asm_op_cmp cmp;
    };
};

struct codegen_asm_data {

    size_t size;
    unsigned char * data;
};

struct codegen_asm {

    enum codegen_asm_type _type;

    union {

        char * comment;
        char * label;
        struct codegen_asm_op op;
        struct codegen_asm_data data;
        char * label_data;
    };
};

struct codegen_asm_list {

    size_t size;
    size_t capacity;
    struct codegen_asm * values;
};

struct codegen_asm codegen_asm_init_comment(char * comment);
struct codegen_asm codegen_asm_init_label(char * label);
struct codegen_asm codegen_asm_init_op(enum codegen_asm_op_opcode opcode);
struct codegen_asm codegen_asm_init_data(size_t size, unsigned char * data);
struct codegen_asm codegen_asm_init_label_data(char * label);
struct codegen_asm codegen_asm_clone(struct codegen_asm value);
void codegen_asm_fini(struct codegen_asm * value);

void codegen_asm_print(struct codegen_asm value, FILE * file);

struct codegen_asm_list codegen_asm_list_init(void);
void codegen_asm_list_append(struct codegen_asm_list * list, struct codegen_asm value);
void codegen_asm_list_concat(struct codegen_asm_list * list, struct codegen_asm_list * append);
struct codegen_asm_list codegen_asm_list_clone(struct codegen_asm_list list);
void codegen_asm_list_fini(struct codegen_asm_list * list);

void codegen_asm_list_print(struct codegen_asm_list value, FILE * file);
