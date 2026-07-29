#ifndef CHAIN_H
#define CHAIN_H

#include "core/command.h"

typedef enum {
    CHAIN_OP_NONE, // first segment on the line; always runs
    CHAIN_OP_AND, // '&&': previous segment, runs only if it exited 0
    CHAIN_OP_OR, // '||': runs only if the previous segment exited non-zero
    CHAIN_OP_SEQ // ';': always runs, regardless of the previous segment
} chain_op_t;

typedef struct {
    char* text; // NUL-terminated pipeline text for this segment (points into line)
    chain_op_t op; // operator that preceded this segment
} chain_segment_t;

// Splits line in place on '&&', '||', and ';' into up to max_segments
// pipeline segments, writing '\0' at each operator and recording which
// operator preceded each resulting segment. A single '&' (background) or
// '|' (pipe) is left untouched, only the doubled forms are treated as
// chain operators. Does not track quoting, so an operator sequence inside
// quotes is still treated as a split point (same simplification the '|'
// pipeline split already makes). Returns the number of segments found.
int split_chain(char* line, chain_segment_t segments[], int max_segments);

#endif
