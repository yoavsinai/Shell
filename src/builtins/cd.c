#include "builtins/cd.h"
#include "core/exit_status.h"
#include <errno.h>

builtin_result_t builtin_cd(struct Command* pipeline, const struct passwd* pw)
{
    const char* target_dir = pipeline[0].argv[1] ? pipeline[0].argv[1] : pw->pw_dir;
    if (chdir(target_dir) < 0) {
        last_exit_status = errno;
        perror("chdir failed");
    } else {
        last_exit_status = 0;
    }
    return BUILTIN_HANDLED;
}
