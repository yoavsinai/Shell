#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"
#include "jobs.h"
#include <signal.h>

// Wires adjacent stages together with pipe(2): stage i's stdout feeds
// stage i+1's stdin, unless either side already has an explicit file
// redirect, in which case that redirect wins and the pipe end is closed.
void connect_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds);

// Forks one child per stage, wires up in_fd/out_fd via dup2, execvp's the
// stage's argv, and waits for all children to finish.
void execute_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds, char cmd_line[LINE_BUFFER_SIZE]);

#endif
