#include "builtins/jobs.h"
#include "core/exit_status.h"
#include "jobs/job_table.h"

static const char* job_state_str(job_state_t state)
{
    switch (state) {
    case JOB_RUNNING:
        return "Running";
    case JOB_STOPPED:
        return "Stopped";
    default:
        return "Done";
    }
}

builtin_result_t builtin_jobs(void)
{
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].in_use) {
            printf("[%d] %s\t%s\n", jobs[i].job_id, job_state_str(jobs[i].state), jobs[i].cmd_line);
        }
    }
    last_exit_status = 0;
    return BUILTIN_HANDLED;
}
