#include "builtins/fg.h"
#include "builtins/job_id.h"
#include "exit_status.h"
#include "jobs.h"
#include <signal.h>

builtin_result_t builtin_fg(struct Command* pipeline)
{
    if (pipeline[0].argv[1] == NULL) {
        fprintf(stderr, "fg: job id required\n");
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }
    int job_id;
    if (!parse_job_id(pipeline[0].argv[1], "fg", &job_id)) {
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }
    job_t* job = find_job(job_id);
    if (job == NULL) {
        fprintf(stderr, "fg: no such job\n");
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }

    tcsetpgrp(STDIN_FILENO, job->pgid);
    kill(-job->pgid, SIGCONT);
    job->state = JOB_RUNNING;
    wait_for_job(job);
    tcsetpgrp(STDIN_FILENO, getpgrp());

    // wait_for_job() doesn't capture individual exit codes, so "$?" only
    // distinguishes stopped (1) from run-to-completion (0) here, rather
    // than reflecting the job's real exit status.
    last_exit_status = (job->state == JOB_STOPPED) ? 1 : 0;
    if (job->state != JOB_STOPPED) {
        job->in_use = 0;
    }
    return BUILTIN_HANDLED;
}
