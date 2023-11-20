#pragma once

#include "flow_graph.h"
#include "asm.h"


extern const char * const codegen_header;
extern const char * const codegen_footer;
extern const char * const codegen_builtins;

struct codegen_asm_list codegen_generate(struct flow_graph_subroutine_list subroutines);
