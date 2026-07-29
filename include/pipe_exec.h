#ifndef PIPE_EXEC_H
#define PIPE_EXEC_H

#include "command.h"

// Forks one child per stage, wires up in_fd/out_fd via dup2, execvp's the
// stage's argv, and either waits for all children to finish (foreground)
// or registers them as a background job (trailing "&") via add_background_job().
void execute_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds, char cmd_line[LINE_BUFFER_SIZE]);

#endif
