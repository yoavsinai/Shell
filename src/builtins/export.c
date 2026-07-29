#include "builtins/export.h"
#include "core/exit_status.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

builtin_result_t builtin_export(struct Command* pipeline)
{
    char** argv = pipeline[0].argv;
    if (argv[1] == NULL) {
        fprintf(stderr, "export: usage: export NAME=VALUE\n");
        last_exit_status = EINVAL;
        return BUILTIN_HANDLED;
    }

    int status = 0;
    for (int i = 1; argv[i] != NULL; i++) {
        char* eq = strchr(argv[i], '=');
        if (eq == NULL) {
            fprintf(stderr, "export: usage: export NAME=VALUE\n");
            status = EINVAL;
            continue;
        }

        *eq = '\0';
        const char* name = argv[i];
        const char* value = eq + 1;
        if (setenv(name, value, 1) < 0) {
            status = errno;
            perror("export failed");
        }
        *eq = '=';
    }
    last_exit_status = status;
    return BUILTIN_HANDLED;
}
