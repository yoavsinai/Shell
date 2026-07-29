#include "builtins/fg.h"
#include "builtins/job_id.h"
#include "core/exit_status.h"
#include "jobs/job_table.h"
#include <errno.h>
#include <signal.h>

builtin_result_t builtin_fg(struct Command* pipeline)
{
    if (pipeline[0].argv[1] == NULL) {
        fprintf(stderr, "fg: job id required\n");
        last_exit_status = EINVAL;
        return BUILTIN_HANDLED;
    }
    int job_id;
    if (!parse_job_id(pipeline[0].argv[1], "fg", &job_id)) {
        last_exit_status = errno;
        return BUILTIN_HANDLED;
    }
    job_t* job = find_job(job_id);
    if (job == NULL) {
        fprintf(stderr, "fg: no such job\n");
        last_exit_status = ESRCH;
        return BUILTIN_HANDLED;
    }

    tcsetpgrp(STDIN_FILENO, job->pgid);
    kill(-job->pgid, SIGCONT);
    job->state = JOB_RUNNING;
    int exit_status = 0;
    wait_for_job(job, &exit_status);
    tcsetpgrp(STDIN_FILENO, getpgrp());

    // Match the shell convention of 128+signal for a job that stops again
    // instead of running to completion.
    last_exit_status = (job->state == JOB_STOPPED) ? 128 + SIGTSTP : exit_status;
    if (job->state != JOB_STOPPED) {
        job->in_use = 0;
    }
    return BUILTIN_HANDLED;
}
