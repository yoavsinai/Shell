#include "builtins.h"
#include "jobs.h"

builtin_result_t run_builtin(struct Command* pipeline, const char* history_path, const struct passwd* pw)
{
    const char* cmd = pipeline[0].argv[0];

    if (strcmp(cmd, "!exit") == 0)
        return BUILTIN_EXIT;

    if (strcmp(cmd, "cd") == 0) {
        const char* target_dir = pipeline[0].argv[1] ? pipeline[0].argv[1] : pw->pw_dir;
        if (chdir(target_dir) < 0) {
            perror("chdir failed");
        }
        return BUILTIN_HANDLED;
    }

    if (strcmp(cmd, "history") == 0) {
        FILE* history_file = fopen(history_path, "r");
        if (history_file == NULL) {
            perror("fopen history file failed");
            return BUILTIN_HANDLED;
        }
        char history_line[LINE_BUFFER_SIZE];
        int line_number = 1;
        while (fgets(history_line, sizeof(history_line), history_file)) {
            printf("%d: %s", line_number++, history_line);
        }
        fclose(history_file);
        return BUILTIN_HANDLED;
    }

    if (strcmp(cmd, "jobs") == 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].in_use) {
                const char* state_str = (jobs[i].state == JOB_RUNNING) ? "Running" : (jobs[i].state == JOB_STOPPED) ? "Stopped"
                                                                                                                     : "Done";
                printf("[%d] %s\t%s\n", jobs[i].job_id, state_str, jobs[i].cmd_line);
            }
        }
        return BUILTIN_HANDLED;
    }

    return BUILTIN_NOT_MATCHED;
}
