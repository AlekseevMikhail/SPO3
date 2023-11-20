#pragma once

#include <stdlib.h>


__attribute__((noreturn))
static inline void unreachable(void) {
    abort();
}
