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

// Finds a free slot in jobs[] and registers the job with the given process
// group, member pids, and source command line, in JOB_RUNNING state.
// Returns the slot index on success, -1 if the job table is full (already
// reported to stderr).
int add_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE]);

// add_job() plus the "[job_id] pgid" line printed for a new background job.
// Returns the slot index, or -1 if the job table is full.
int add_background_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE]);

// add_job() plus marking the job JOB_STOPPED and printing the "[job_id]+
// Stopped" line. Returns the slot index, or -1 if the job table is full.
int add_stopped_job(pid_t pgid, pid_t pids[MAX_PIPELINE_STAGES], int num_pids, const char cmd_line[LINE_BUFFER_SIZE]);

// Returns the in-use job with the given job_id, or NULL if there isn't one.
job_t* find_job(int job_id);

// Waits for every process in the job's pipeline to either exit or stop,
// and updates job->state accordingly (JOB_STOPPED or JOB_DONE). A SIGTSTP
// stops the whole process group at once, but waitpid() only reports one
// event per call, so this waits once per member pid (on the group, not a
// single pid) to collect them all.
void wait_for_job(job_t* job);

#endif