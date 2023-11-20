#pragma once

#include "flow_graph.h"
#include "source.h"
#include "error.h"
#include "ast.h"


void ast_analyze(
        const struct ast_analyze_source_list * sources,
        struct flow_graph_subroutine_list * subroutines,
        struct ast_analyze_error_list * errors
);
