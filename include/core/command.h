#ifndef COMMAND_H
#define COMMAND_H

#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LINE_BUFFER_SIZE 512

#define MAX_HISTORY 100

// Max argv a single command can take (including the NULL terminator).
#define MAX_ARGS LINE_BUFFER_SIZE

// Max number of commands chained with '|' in one pipeline.
#define MAX_PIPELINE_STAGES LINE_BUFFER_SIZE

// Max number of pipelines chained with '&&', '||', or ';' on one line.
#define MAX_CHAIN_SEGMENTS LINE_BUFFER_SIZE

// One stage of a pipeline: an argv plus the fds it reads from / writes to.
// in_fd/out_fd default to STDIN_FILENO/STDOUT_FILENO and are only replaced
// by explicit '<'/'>'/'>>' redirection or by connect_pipeline().
struct Command {
    char* argv[MAX_ARGS];
    int argc;
    int in_fd;
    int out_fd;
};

#endif
