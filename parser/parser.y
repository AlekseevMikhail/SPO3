%parse-param {struct ast_source ** result} {char ** error}

%{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "utils/mallocs.h"

int yylex(void);

void yyerror(struct ast_source ** result, char ** error, const char * str);

#define POS position_init(yylocp->first_line, yylocp->first_column)
%}

%locations
%glr-parser

%token T_IDENTIFIER T_STR T_CHAR T_HEX T_BITS T_DEC T_BOOL
%token TL_BOOL "bool"
%token TL_BYTE "byte"
%token TL_INT "int"
%token TL_UINT "uint"
%token TL_LONG "long"
%token TL_ULONG "ulong"
%token TL_CHAR "char"
%token TL_STRING "string"
%token TL_IF "if"
%token TL_ELSE "else"
%token TL_WHILE "while"
%token TL_DO "do"
%token TL_BREAK "break"
%token TL_AND "&&"
%token TL_OR "||"
%token TL_EQ "=="
%token TL_NE "!="
%token TL_LE "<="
%token TL_GE ">="
%token TL_LBS "<<"
%token TL_RBS ">>"
%token TL_PLUS_EQ "+="
%token TL_MINUS_EQ "-="
%token TL_MULTIPLY_EQ "*="
%token TL_DIVIDE_EQ "/="
%token TL_REMAINDER_EQ "%="
%token TL_BAND_EQ "&="
%token TL_BOR_EQ "|="
%token TL_BXOR_EQ "^="
%token TL_AND_EQ "&&="
%token TL_OR_EQ "||="
%token TL_INC "++"
%token TL_DEC "--"

%precedence P_IF
%left "else"
%nonassoc '=' "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" "&&=" "||="
%left "||"
%left "&&"
%nonassoc "==" "!=" '<' "<=" '>' ">="
%left "<<" ">>"
%left '|' '^'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%precedence P_UNARY
%precedence '[' '('

%code requires {
#include "ast.h"
}

%union {
    struct ast_expr * expr;
    struct ast_expr_list expr_list;
    struct ast_function_signature * function_signature;
    struct ast_function_signature_arg function_signature_arg;
    struct ast_function_signature_arg_list function_signature_arg_list;
    struct ast_literal * literal;
    struct ast_source * source;
    struct ast_source_item * source_item;
    struct ast_stmt * stmt;
    struct ast_stmt_list stmt_list;
    struct ast_stmt_var_id stmt_var_id;
    struct ast_stmt_var_id_list stmt_var_id_list;
    struct ast_type_reference * type_reference;
    char * token;
    size_t _int;
}

%type<expr> expr expr_binary expr_unary expr_braces expr_call expr_indexer expr_place expr_literal
%type<expr_list> expr_list expr_list1
%type<function_signature> function_signature
%type<function_signature_arg> function_signature_arg
%type<function_signature_arg_list> function_signature_arg_list function_signature_arg_list1
%type<literal> literal literal_bool literal_str literal_char literal_hex literal_bits literal_dec
%type<source> source
%type<source_item> source_item source_item_func_decl
%type<stmt> stmt stmt_var stmt_if stmt_block stmt_while stmt_do stmt_break stmt_expr
%type<stmt_list> stmt_block_stmts
%type<stmt_var_id> stmt_var_id
%type<stmt_var_id_list> stmt_var_id_list
%type<type_reference> type_reference type_reference_opt
    type_reference_builtin type_reference_custom type_reference_array
%type<token> T_IDENTIFIER T_STR T_CHAR T_HEX T_BITS T_DEC T_BOOL
%type<_int> type_reference_array_commas

%%

file
    : source YYEOF  { *result = $1; }
    ;

source
    : /* empty */           { $$ = ast_source_new(POS); }
    | source source_item    { $$ = $1; ast_source_item_list_append(&$1->items, $2); }
    ;

source_item
    : source_item_func_decl
    ;

source_item_func_decl
    : function_signature ';'        { $$ = ast_source_item_new_func_decl(POS, $1, NULL); }
    | function_signature stmt_block { $$ = ast_source_item_new_func_decl(POS, $1, $2); }
    ;

function_signature
    : type_reference_opt T_IDENTIFIER '(' function_signature_arg_list ')' {
        $$ = ast_function_signature_new(POS, $1, $2, $4);
    }
    ;

function_signature_arg_list
    : /* empty */                   { $$ = ast_function_signature_arg_list_init(); }
    | function_signature_arg_list1
    ;

function_signature_arg_list1
    : function_signature_arg    {
        $$ = ast_function_signature_arg_list_init();
        ast_function_signature_arg_list_append(&$$, $1);
    }
    | function_signature_arg_list1 ',' function_signature_arg {
        $$ = $1;
        ast_function_signature_arg_list_append(&$$, $3);
    }
    ;

function_signature_arg
    : type_reference_opt T_IDENTIFIER   { $$ = ast_function_signature_arg_init(POS, $1, $2); }
    ;

type_reference_opt
    : /* empty */       { $$ = NULL; }
    | type_reference
    ;

type_reference
    : type_reference_builtin
    | type_reference_custom
    | type_reference_array
    ;

type_reference_builtin
    : "bool"    { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_BOOL); }
    | "byte"    { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_BYTE); }
    | "int"     { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_INT); }
    | "uint"    { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_UINT); }
    | "long"    { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_LONG); }
    | "ulong"   { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_ULONG); }
    | "char"    { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_CHAR); }
    | "string"  { $$ = ast_type_reference_new_builtin(POS, AST_TYPE_REFERENCE_BUILTIN_TYPE_STRING); }
    ;

type_reference_custom
    : T_IDENTIFIER  { $$ = ast_type_reference_new_custom(POS, $1); }
    ;

type_reference_array
    : type_reference '[' type_reference_array_commas ']'    {
        $$ = ast_type_reference_new_array(POS, $1, $3);
    }
    ;

type_reference_array_commas
    : /* empty */                       { $$ = 1; }
    | type_reference_array_commas ','   { $$ = $1 + 1; }
    ;

stmt
    : stmt_var
    | stmt_if
    | stmt_block
    | stmt_while
    | stmt_do
    | stmt_break
    | stmt_expr
    ;

stmt_var
    : type_reference stmt_var_id_list ';'   { $$ = ast_stmt_new_var(POS, $1, $2); }
    ;

stmt_var_id_list
    : stmt_var_id   {
        $$ = ast_stmt_var_id_list_init();
        ast_stmt_var_id_list_append(&$$, $1);
    }
    | stmt_var_id_list ',' stmt_var_id  {
        $$ = $1;
        ast_stmt_var_id_list_append(&$$, $3);
    }
    ;

stmt_var_id
    : T_IDENTIFIER          { $$ = ast_stmt_var_id_init(POS, $1, NULL); }
    | T_IDENTIFIER '=' expr { $$ = ast_stmt_var_id_init(POS, $1, $3); }
    ;

stmt_if
    : "if" '(' expr ')' stmt %prec P_IF     { $$ = ast_stmt_new_if(POS, $3, $5, NULL); }
    | "if" '(' expr ')' stmt "else" stmt    { $$ = ast_stmt_new_if(POS, $3, $5, $7); }
    ;

stmt_block
    : '{' stmt_block_stmts '}'  { $$ = ast_stmt_new_block(POS, $2); }
    ;

stmt_block_stmts
    : /* empty */           { $$ = ast_stmt_list_init(); }
    | stmt_block_stmts stmt { $$ = $1; ast_stmt_list_append(&$$, $2); }
    ;

stmt_while
    : "while" '(' expr ')' stmt { $$ = ast_stmt_new_while(POS, $3, $5); }
    ;

stmt_do
    : "do" stmt_block "while" '(' expr ')' ';'  { $$ = ast_stmt_new_do(POS, $2, $5); }
    ;

stmt_break
    : "break" ';'   { $$ = ast_stmt_new_break(POS); }
    ;

stmt_expr
    : expr ';'  { $$ = ast_stmt_new_expr(POS, $1); }
    ;

expr
    : expr_binary
    | expr_unary
    | expr_braces
    | expr_call
    | expr_indexer
    | expr_place
    | expr_literal
    ;

expr_binary
    : expr '=' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT, $1, $3); }
    | expr '+' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_PLUS, $1, $3); }
    | expr '-' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_MINUS, $1, $3); }
    | expr '*' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_MULTIPLY, $1, $3); }
    | expr '/' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_DIVIDE, $1, $3); }
    | expr '%' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_REMAINDER, $1, $3); }
    | expr '&' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_BITWISE_AND, $1, $3); }
    | expr '|' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_BITWISE_OR, $1, $3); }
    | expr '^' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_BITWISE_XOR, $1, $3); }
    | expr "&&" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_AND, $1, $3); }
    | expr "||" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_OR, $1, $3); }
    | expr "==" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_EQ, $1, $3); }
    | expr "!=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_NE, $1, $3); }
    | expr '<' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_LT, $1, $3); }
    | expr "<=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_LE, $1, $3); }
    | expr '>' expr     { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_GT, $1, $3); }
    | expr ">=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_GE, $1, $3); }
    | expr "<<" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_LEFT_BITSHIFT, $1, $3); }
    | expr ">>" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_RIGHT_BITSHIFT, $1, $3); }
    | expr "+=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_PLUS, $1, $3); }
    | expr "-=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_MINUS, $1, $3); }
    | expr "*=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_MULTIPLY, $1, $3); }
    | expr "/=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_DIVIDE, $1, $3); }
    | expr "%=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_REMAINDER, $1, $3); }
    | expr "&=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_AND, $1, $3); }
    | expr "|=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_OR, $1, $3); }
    | expr "^=" expr    { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_BITWISE_XOR, $1, $3); }
    | expr "&&=" expr   { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_AND, $1, $3); }
    | expr "||=" expr   { $$ = ast_expr_new_binary(POS, AST_EXPR_BINARY_OP_ASSIGNMENT_OR, $1, $3); }
    ;

expr_unary
    : '-' expr %prec P_UNARY    { $$ = ast_expr_new_unary(POS, AST_EXPR_UNARY_OP_MINUS, $2); }
    | '~' expr %prec P_UNARY    { $$ = ast_expr_new_unary(POS, AST_EXPR_UNARY_OP_BITWISE_NOT, $2); }
    | '!' expr %prec P_UNARY    { $$ = ast_expr_new_unary(POS, AST_EXPR_UNARY_OP_NOT, $2); }
    | "++" expr %prec P_UNARY   { $$ = ast_expr_new_unary(POS, AST_EXPR_UNARY_OP_INC, $2); }
    | "--" expr %prec P_UNARY   { $$ = ast_expr_new_unary(POS, AST_EXPR_UNARY_OP_DEC, $2); }
    ;

expr_braces
    : '(' expr ')'  { $$ = ast_expr_new_braces(POS, $2); }
    ;

expr_call
    : expr '(' expr_list ')'    { $$ = ast_expr_new_call(POS, $1, $3); }
    ;

expr_indexer
    : expr '[' expr_list1 ']'   { $$ = ast_expr_new_indexer(POS, $1, $3); }
    ;

expr_place
    : T_IDENTIFIER  { $$ = ast_expr_new_place(POS, $1); }
    ;

expr_literal
    : literal   { $$ = ast_expr_new_literal(POS, $1); }
    ;

expr_list
    : /* empty */   { $$ = ast_expr_list_init(); }
    | expr_list1
    ;

expr_list1
    : expr                  {
        $$ = ast_expr_list_init();
        ast_expr_list_append(&$$, $1);
    }
    | expr_list1 ',' expr   {
        $$ = $1;
        ast_expr_list_append(&$$, $3);
    }
    ;

literal
    : literal_bool
    | literal_str
    | literal_char
    | literal_hex
    | literal_bits
    | literal_dec
    ;

literal_bool
    : T_BOOL    { $$ = ast_literal_new_bool(POS, $1[0] == 't'); free($1); }
    ;

literal_str
    : T_STR     { $$ = ast_literal_new_str(POS, $1); }
    ;

literal_char
    : T_CHAR    { $$ = ast_literal_new_char(POS, $1[1]); free($1); }
    ;

literal_hex
    : T_HEX     { $$ = ast_literal_new_hex(POS, $1); }
    ;

literal_bits
    : T_BITS    { $$ = ast_literal_new_bits(POS, $1); }
    ;

literal_dec
    : T_DEC     { $$ = ast_literal_new_dec(POS, $1); }
    ;

%%

void yyerror(struct ast_source ** result, char ** error, const char * str) {
    free(*error);

    *error = mallocs(strlen(str) + 56);
    sprintf(*error, "at %d:%d: %s", yylloc.first_line, yylloc.first_column, str);
}
