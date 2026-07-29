#include "pipe_exec.h"
#include "exit_status.h"
#include "jobs.h"
#include <signal.h>

void execute_pipeline(struct Command pipeline[MAX_PIPELINE_STAGES], int num_cmds, char cmd_line[LINE_BUFFER_SIZE])
{
    short background = 0;
    struct Command* last_cmd = &pipeline[num_cmds - 1];
    if (strcmp(last_cmd->argv[last_cmd->argc - 1], "&") == 0) {
        background = 1;
        last_cmd->argv[last_cmd->argc - 1] = NULL;
    }

    pid_t pids[MAX_PIPELINE_STAGES];

    pid_t pgid = 0; // Process group ID for the pipeline

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

            setpgid(0, pgid); // Set the child process group ID to its own PID
            signal(SIGINT, SIG_DFL); // Restore default signal handling for SIGINT in the child process

            execvp(pipeline[i].argv[0], pipeline[i].argv);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }

        pids[i] = pid;

        if (pgid == 0) {
            pgid = pid; // Set the process group ID to the first child's PID
        }
        setpgid(pid, pgid); // Set the child process group ID to the pipeline's PID

        // Parent must also close its copies, or the pipe never sees EOF.
        if (pipeline[i].in_fd != STDIN_FILENO)
            close(pipeline[i].in_fd);
        if (pipeline[i].out_fd != STDOUT_FILENO)
            close(pipeline[i].out_fd);
    }

    if (!background) {
        tcsetpgrp(STDIN_FILENO, pgid); // Set the terminal's foreground process group to the pipeline's PID

        // A SIGTSTP stops every process in the group at once, but waitpid()
        // only reports one event per call, so wait once per pipeline stage
        // (on the group, not a single pid) to collect them all instead of
        // stopping early and leaving later stages unwaited.
        int stopped = 0;
        for (int i = 0; i < num_cmds; i++) {
            int status = 0;
            pid_t waited_pid = waitpid(-pgid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                stopped = 1;
            } else if (waited_pid == pids[num_cmds - 1]) {
                // "$?" reflects the exit status of the last pipeline stage.
                last_exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
            }
        }
        if (stopped)
            add_stopped_job(pgid, pids, num_cmds, cmd_line);

        tcsetpgrp(STDIN_FILENO, getpgrp()); // Restore the terminal's foreground process group to the shell
    } else {
        add_background_job(pgid, pids, num_cmds, cmd_line);
    }
}
