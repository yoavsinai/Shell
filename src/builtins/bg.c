#include "builtins/bg.h"
#include "builtins/job_id.h"
#include "exit_status.h"
#include "jobs.h"
#include <signal.h>

builtin_result_t builtin_bg(struct Command* pipeline)
{
    if (pipeline[0].argv[1] == NULL) {
        fprintf(stderr, "bg: job id required\n");
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }
    int job_id;
    if (!parse_job_id(pipeline[0].argv[1], "bg", &job_id)) {
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }
    job_t* job = find_job(job_id);
    if (job == NULL) {
        fprintf(stderr, "bg: no such job\n");
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }

    kill(-job->pgid, SIGCONT);
    job->state = JOB_RUNNING;
    printf("[%d] %s &\n", job->job_id, job->cmd_line);
    last_exit_status = 0;
    return BUILTIN_HANDLED;
}
