#ifndef PIPELINE_DISPATCH_H
#define PIPELINE_DISPATCH_H

#include "core/command.h"
#include <pwd.h>

// Parses and runs one chain segment (a "cmd | cmd | ..." pipeline already
// split off from any &&/||/; chaining): splits it on '|', parses each
// stage, and either dispatches to a builtin or wires up and forks the
// pipeline. line is the original raw input line, used only as the
// job-table label if the segment backgrounds.
// Returns 1 if the REPL should exit ("!exit" was entered), 0 otherwise.
int dispatch_pipeline(char* segment, char* line, const char* history_path, const struct passwd* pw);

#endif
