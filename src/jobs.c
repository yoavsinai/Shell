#include "jobs.h"

job_t jobs[MAX_JOBS];
int next_job_id = 1;

int add_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE])
{
    int slot = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].in_use) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum number of jobs reached\n");
        return -1;
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
    return slot;
}

int add_background_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE])
{
    int slot = add_job(pgid, pids, num_pids, cmd_line);
    if (slot < 0)
        return -1;
    printf("[%d] %d\n", jobs[slot].job_id, pgid);
    return slot;
}

int add_stopped_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE])
{
    int slot = add_job(pgid, pids, num_pids, cmd_line);
    if (slot < 0)
        return -1;
    jobs[slot].state = JOB_STOPPED;
    printf("[%d]+ Stopped\t%s\n", jobs[slot].job_id, jobs[slot].cmd_line);
    return slot;
}

job_t* find_job(int job_id)
{
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].in_use && jobs[i].job_id == job_id) {
            return &jobs[i];
        }
    }
    return NULL;
}

void wait_for_job(job_t* job)
{
    int stopped = 0;
    for (int i = 0; i < job->num_pids; i++) {
        int status = 0;
        waitpid(-job->pgid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            stopped = 1;
        }
    }
    job->state = stopped ? JOB_STOPPED : JOB_DONE;
}
