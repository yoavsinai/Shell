#include "builtins/builtins.h"
#include "builtins/bg.h"
#include "builtins/cd.h"
#include "builtins/export.h"
#include "builtins/fg.h"
#include "builtins/history.h"
#include "builtins/jobs.h"
#include "builtins/kill.h"

builtin_result_t run_builtin(struct Command* pipeline, const char* history_path, const struct passwd* pw)
{
    const char* cmd = pipeline[0].argv[0];

    if (strcmp(cmd, "!exit") == 0)
        return BUILTIN_EXIT;
    if (strcmp(cmd, "cd") == 0)
        return builtin_cd(pipeline, pw);
    if (strcmp(cmd, "export") == 0)
        return builtin_export(pipeline);
    if (strcmp(cmd, "history") == 0)
        return builtin_history(history_path);
    if (strcmp(cmd, "jobs") == 0)
        return builtin_jobs();
    if (strcmp(cmd, "fg") == 0)
        return builtin_fg(pipeline);
    if (strcmp(cmd, "bg") == 0)
        return builtin_bg(pipeline);
    if (strcmp(cmd, "kill") == 0)
        return builtin_kill(pipeline);

    return BUILTIN_NOT_MATCHED;
}
