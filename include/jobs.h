#ifndef JOBS_H
#define JOBS_H

#include "command.h"

#define MAX_JOBS 100

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_state_t;

typedef struct {
    int job_id;
    pid_t pgid;
    pid_t pids[MAX_PIPELINE_STAGES];
    int num_pids;
    char cmd_line[LINE_BUFFER_SIZE];
    job_state_t state;
    int in_use;
} job_t;

extern job_t jobs[MAX_JOBS];
extern int next_job_id;

#endif