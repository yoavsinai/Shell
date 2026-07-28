#include "executor.h"

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

void execute_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds)
{
    short background = 0;
    struct Command* last_cmd = &pipeline[num_cmds - 1];
    if (strcmp(last_cmd->argv[last_cmd->argc - 1], "&") == 0) {
        background = 1;
        last_cmd->argv[last_cmd->argc - 1] = NULL;
    }
    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return;
        }

        if (pid == 0) {
            if (pipeline[i].in_fd != STDIN_FILENO) {
                dup2(pipeline[i].in_fd, STDIN_FILENO);
                close(pipeline[i].in_fd);
            }

            if (pipeline[i].out_fd != STDOUT_FILENO) {
                dup2(pipeline[i].out_fd, STDOUT_FILENO);
                close(pipeline[i].out_fd);
            }

            execvp(pipeline[i].argv[0], pipeline[i].argv);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }

        // Parent must also close its copies, or the pipe never sees EOF.
        if (pipeline[i].in_fd != STDIN_FILENO)
            close(pipeline[i].in_fd);
        if (pipeline[i].out_fd != STDOUT_FILENO)
            close(pipeline[i].out_fd);
    }

    if (!background)
        for (int i = 0; i < num_cmds; i++) {
            wait(NULL);
        }
}
