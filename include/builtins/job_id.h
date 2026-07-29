#ifndef BUILTINS_JOB_ID_H
#define BUILTINS_JOB_ID_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// Parses arg as a positive job id for fg/bg. Prints "<name>: invalid job
// id" and returns 0 on any parse failure instead of trusting errno alone,
// since strtol() leaves it untouched on plain non-numeric input.
static inline int parse_job_id(const char* arg, const char* builtin_name, int* out_job_id)
{
    char* endptr;
    errno = 0;
    long job_id = strtol(arg, &endptr, 10);
    if (errno != 0 || endptr == arg || *endptr != '\0' || job_id <= 0) {
        fprintf(stderr, "%s: invalid job id\n", builtin_name);
        return 0;
    }
    *out_job_id = (int)job_id;
    return 1;
}

#endif
