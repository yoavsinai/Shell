#include "line/pipeline_dispatch.h"
#include "builtins/builtins.h"
#include "core/exit_status.h"
#include "parsing/parse_command.h"
#include "exec/pipe_connect.h"
#include "exec/pipe_exec.h"
#include <errno.h>

int dispatch_pipeline(char* segment, char* line, const char* history_path, const struct passwd* pw)
{
    static struct Command pipeline[MAX_PIPELINE_STAGES];
    char pipe_delim[] = "|";
    int num_cmds = 0;
    char* pipe_saveptr = NULL;
    char* stage = strtok_r(segment, pipe_delim, &pipe_saveptr);
    while (stage != NULL && num_cmds < MAX_PIPELINE_STAGES - 1) {
        if (parse_command(stage, &pipeline[num_cmds++])) {
            fprintf(stderr, "Error parsing command: %s\n", stage);
            return 0;
        }
        stage = strtok_r(NULL, pipe_delim, &pipe_saveptr);
    }

    if (num_cmds == MAX_PIPELINE_STAGES - 1 && stage != NULL) {
        fprintf(stderr, "Error: Too many commands in pipeline (max %d)\n", MAX_PIPELINE_STAGES);
        last_exit_status = E2BIG;
        return 0;
    }

    if (num_cmds == 0 || pipeline[0].argv[0] == NULL)
        return 0;

    for (int i = 0; i < num_cmds; i++) {
        if (pipeline[i].argv[0] == NULL) {
            fprintf(stderr, "Error: empty command in pipeline\n");
            last_exit_status = EINVAL;
            return 0;
        }
    }

    builtin_result_t builtin_result = run_builtin(pipeline, history_path, pw);
    if (builtin_result == BUILTIN_EXIT)
        return 1;
    if (builtin_result == BUILTIN_HANDLED)
        return 0;

    connect_pipeline(pipeline, num_cmds);
    execute_pipeline(pipeline, num_cmds, line);
    return 0;
}
