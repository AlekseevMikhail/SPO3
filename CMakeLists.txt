cmake_minimum_required(VERSION 3.22.1)
project(spo C)

set(CMAKE_C_STANDARD 11)

find_package(BISON)
find_package(FLEX)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-Wall -Werror -pedantic-errors)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

FLEX_TARGET(Lexer parser/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/parser/lexer.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/lexer.h)

BISON_TARGET(Parser parser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.c
        COMPILE_FLAGS -Wcounterexamples
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.h
        VERBOSE REPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.txt)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_executable(display
        ast/expr.c
        ast/expr.h
        ast/function_signature.c
        ast/function_signature.h
        ast/literal.h
        ast/source.h
        ast/source_item.h
        ast/stmt.h
        ast/type_reference.h
        ast.h
        main_display.c
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
        ast/literal.c
        ast/source.c
        ast/source_item.c
        ast/stmt.c
        ast/type_reference.c
        ast_display.h
        ast_display.c
        utils/position.h
)

add_executable(analyze
        ast/expr.c
        ast/expr.h
        ast/function_signature.c
        ast/function_signature.h
        ast/literal.h
        ast/source.h
        ast/source_item.h
        ast/stmt.h
        ast/type_reference.h
        ast.h
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
        ast/literal.c
        ast/source.c
        ast/source_item.c
        ast/stmt.c
        ast/type_reference.c
        ast_display.h
        ast_display.c
        utils/position.h
        flow_graph/subroutine.h
        flow_graph/local.h
        flow_graph/node.h
        ast_analyze/analyze.c
        flow_graph.h
        ast_analyze/source.h
        ast_analyze/error.h
        ast_analyze/analyze.h
        ast_analyze/reference.h
        ast_analyze/context.h
        main_analyze.c
        ast_analyze/error.c
        flow_graph/subroutine.c
        ast_analyze/context.c
        ast_analyze/reference.c
        ast_analyze/source.c
        flow_graph/local.c
        flow_graph/node.c
        utils/mallocs.h
        utils/unreachable.h
        flow_graph/expr.h
        flow_graph/literal.h
        flow_graph/literal.c
        flow_graph/expr.c
        flow_graph_display.h
        flow_graph_display.c
        codegen/asm.h
        codegen/generate.h
        codegen/asm.c
        codegen/generate.c
)
