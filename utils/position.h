#pragma once

#include "stdlib.h"


struct position {
    size_t row;
    size_t column;
};


static inline struct position position_init(size_t row, size_t column) {
    return (struct position) {
        .row = row,
        .column = column,
    };
}
