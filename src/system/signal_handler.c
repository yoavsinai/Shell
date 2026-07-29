#include "system/signal_handler.h"

void sigint_handler(int signo)
{
    return;
}

void sigchld_handler(int signo)
{
    int saved_errno = errno;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (!jobs[i].in_use) {
                continue;
            }
            int all_done = 1;
            for (int p = 0; p < jobs[i].num_pids; p++) {
                if (jobs[i].pids[p] == pid) {
                    jobs[i].pids[p] = -1; // Mark this PID as done
                }
            }
            for (int p = 0; p < jobs[i].num_pids; p++) {
                if (jobs[i].pids[p] != -1) {
                    all_done = 0;
                    break;
                }
            }
            if (all_done) {
                jobs[i].state = JOB_DONE;
            }
        }
        errno = saved_errno;
    }
}