#pragma once

#include <stdlib.h>
#include <stdio.h>


static inline void * mallocs(size_t size) {
    void * const result = malloc(size);

    if (size && !result) {
        fprintf(stderr, "not enough RAM\n");
        abort();
    }

    return result;
}

static inline void * reallocs(void * mem, size_t size) {
    void * const result = realloc(mem, size);

    if (size && !result) {
        fprintf(stderr, "not enough RAM\n");
        abort();
    }

    return result;
}
