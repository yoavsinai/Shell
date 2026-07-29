#include "jobs.h"

job_t jobs[MAX_JOBS];
int next_job_id = 1;

int add_background_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE])
{
    int slot = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].in_use) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of background jobs reached\n");
        return 0;
    }

    jobs[slot].job_id = next_job_id++;
    jobs[slot].pgid = pgid;
    jobs[slot].num_pids = num_pids;
    for (int i = 0; i < num_pids; i++) {
        jobs[slot].pids[i] = pids[i];
    }
    jobs[slot].state = JOB_RUNNING;
    jobs[slot].in_use = 1;
    strncpy(jobs[slot].cmd_line, cmd_line, LINE_BUFFER_SIZE - 1);
    jobs[slot].cmd_line[LINE_BUFFER_SIZE - 1] = '\0';
    printf("[%d] %d\n", jobs[slot].job_id, pgid);
    return 1;
}
