#include "builtins/cd.h"
#include "exit_status.h"

builtin_result_t builtin_cd(struct Command* pipeline, const struct passwd* pw)
{
    const char* target_dir = pipeline[0].argv[1] ? pipeline[0].argv[1] : pw->pw_dir;
    if (chdir(target_dir) < 0) {
        perror("chdir failed");
        last_exit_status = 1;
    } else {
        last_exit_status = 0;
    }
    return BUILTIN_HANDLED;
}
