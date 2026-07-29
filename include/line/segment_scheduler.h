#ifndef SEGMENT_SCHEDULER_H
#define SEGMENT_SCHEDULER_H

#include "core/command.h"
#include <pwd.h>

// Runs one already-normalized input line: splits it into &&/||/;-chained
// chain_segment_t entries (see chain.h), then dispatches each one in turn
// (see pipeline_dispatch.h), short-circuiting &&/|| on last_exit_status
// along the way. line is the original raw input line, used only as the
// job-table label if a segment backgrounds.
// Returns 1 if the REPL should exit ("!exit" was entered), 0 otherwise.
int run_chain_segments(char* normalized_line, char* line, const char* history_path, const struct passwd* pw);

#endif
