#include "pipe_connect.h"

void connect_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds)
{
    int pipefd[2];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }

        // Only hook up the pipe if the user didn't already redirect with '>'/'<'.
        if (pipeline[i].out_fd == STDOUT_FILENO) {
            pipeline[i].out_fd = pipefd[1];
        } else {
            close(pipefd[1]);
        }

        if (pipeline[i + 1].in_fd == STDIN_FILENO) {
            pipeline[i + 1].in_fd = pipefd[0];
        } else {
            close(pipefd[0]);
        }
    }
}
