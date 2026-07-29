#include "line/segment_scheduler.h"
#include "core/exit_status.h"
#include "line/chain.h"
#include "line/pipeline_dispatch.h"

int run_chain_segments(char* normalized_line, char* line, const char* history_path, const struct passwd* pw)
{
    chain_segment_t segments[MAX_CHAIN_SEGMENTS];
    int num_segments = split_chain(normalized_line, segments, MAX_CHAIN_SEGMENTS);
    if (num_segments > MAX_CHAIN_SEGMENTS)
        num_segments = MAX_CHAIN_SEGMENTS;

    // A segment gated by && or || only runs if the segments before it
    // (back to the last ';' or the start of the line) actually ran and
    // passed that gate; ';' unconditionally resets the gate for what
    // follows.
    int should_run = 1;
    for (int i = 0; i < num_segments; i++) {
        if (i > 0) {
            switch (segments[i].op) {
            case CHAIN_OP_SEQ:
                should_run = 1;
                break;
            case CHAIN_OP_AND:
                should_run = should_run && last_exit_status == 0;
                break;
            case CHAIN_OP_OR:
                should_run = should_run && last_exit_status != 0;
                break;
            case CHAIN_OP_NONE:
                break;
            }
        }
        if (should_run && dispatch_pipeline(segments[i].text, line, history_path, pw))
            return 1;
    }
    return 0;
}
