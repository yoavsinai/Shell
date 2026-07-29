#include "line/chain.h"

static int emit_segment(chain_segment_t segments[], int max_segments, int count, char* start, chain_op_t op)
{
    if (count < max_segments) {
        segments[count].text = start;
        segments[count].op = op;
    }
    return count + 1;
}

int split_chain(char* line, chain_segment_t segments[], int max_segments)
{
    int count = 0;
    char* start = line;
    char* p = line;
    chain_op_t next_op = CHAIN_OP_NONE;

    while (*p != '\0') {
        chain_op_t op;
        int op_len;

        if (p[0] == '&' && p[1] == '&') {
            op = CHAIN_OP_AND;
            op_len = 2;
        } else if (p[0] == '|' && p[1] == '|') {
            op = CHAIN_OP_OR;
            op_len = 2;
        } else if (p[0] == ';') {
            op = CHAIN_OP_SEQ;
            op_len = 1;
        } else {
            p++;
            continue;
        }

        count = emit_segment(segments, max_segments, count, start, next_op);
        *p = '\0';
        p += op_len;
        start = p;
        next_op = op;
    }

    return emit_segment(segments, max_segments, count, start, next_op);
}
