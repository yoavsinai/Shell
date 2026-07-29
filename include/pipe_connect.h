#ifndef PIPE_CONNECT_H
#define PIPE_CONNECT_H

#include "command.h"

// Wires adjacent stages together with pipe(2): stage i's stdout feeds
// stage i+1's stdin, unless either side already has an explicit file
// redirect, in which case that redirect wins and the pipe end is closed.
void connect_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds);

#endif
