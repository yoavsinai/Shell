#include "builtins/kill.h"
#include "builtins/job_id.h"
#include "core/exit_status.h"
#include "jobs/job_table.h"
#include <errno.h>
#include <signal.h>
#include <string.h>

typedef struct {
    const char* name;
    int sig;
} signal_name_t;

static const signal_name_t SIGNAL_NAMES[] = {
    { "HUP", SIGHUP },
    { "INT", SIGINT },
    { "QUIT", SIGQUIT },
    { "KILL", SIGKILL },
    { "TERM", SIGTERM },
    { "STOP", SIGSTOP },
    { "CONT", SIGCONT },
    { "TSTP", SIGTSTP },
};

// Parses an optional "-SIGNAL" flag (numeric, e.g. "-9", or by name with or
// without the "SIG" prefix, e.g. "-KILL"/"-SIGKILL") out of argv[1].
// Returns 0 and leaves an error unprinted if the flag looks like a signal
// but isn't one of the recognized ones; the caller reports that.
static int parse_signal_flag(const char* flag, int* out_sig)
{
    char* endptr;
    long sig = strtol(flag, &endptr, 10);
    if (*flag != '\0' && *endptr == '\0') {
        *out_sig = (int)sig;
        return 1;
    }
    if (strncmp(flag, "SIG", 3) == 0) {
        flag += 3;
    }
    for (size_t i = 0; i < sizeof(SIGNAL_NAMES) / sizeof(SIGNAL_NAMES[0]); i++) {
        if (strcmp(flag, SIGNAL_NAMES[i].name) == 0) {
            *out_sig = SIGNAL_NAMES[i].sig;
            return 1;
        }
    }
    return 0;
}

builtin_result_t builtin_kill(struct Command* pipeline)
{
    char** argv = pipeline[0].argv;
    if (argv[1] == NULL) {
        fprintf(stderr, "kill: job id required\n");
        last_exit_status = EINVAL;
        return BUILTIN_HANDLED;
    }

    int sig = SIGTERM;
    int job_id_arg = 1;
    if (argv[1][0] == '-') {
        if (!parse_signal_flag(argv[1] + 1, &sig)) {
            fprintf(stderr, "kill: unknown signal %s\n", argv[1]);
            last_exit_status = EINVAL;
            return BUILTIN_HANDLED;
        }
        job_id_arg = 2;
    }

    if (argv[job_id_arg] == NULL) {
        fprintf(stderr, "kill: job id required\n");
        last_exit_status = EINVAL;
        return BUILTIN_HANDLED;
    }
    int job_id;
    if (!parse_job_id(argv[job_id_arg], "kill", &job_id)) {
        last_exit_status = errno;
        return BUILTIN_HANDLED;
    }
    job_t* job = find_job(job_id);
    if (job == NULL) {
        fprintf(stderr, "kill: no such job\n");
        last_exit_status = ESRCH;
        return BUILTIN_HANDLED;
    }

    if (kill(-job->pgid, sig) < 0) {
        last_exit_status = errno;
        perror("kill failed");
    } else {
        last_exit_status = 0;
    }
    return BUILTIN_HANDLED;
}
