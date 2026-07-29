#include "builtins.h"
#include "command.h"
#include "jobs.h"
#include "lexer.h"
#include "parse_command.h"
#include "pipe_connect.h"
#include "pipe_exec.h"
#include "shell_init.h"

int main()
{
    char line[LINE_BUFFER_SIZE];
    char history_path[LINE_BUFFER_SIZE];
    int history_fd;
    struct passwd* pw = shell_init(history_path, &history_fd);
    if (pw == NULL)
        return 0;

    while (1) {
        char cwd[LINE_BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd failed");
            break;
        }

        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].in_use && jobs[i].state == JOB_DONE) {
                printf("[%d] Done\t%s\n", jobs[i].job_id, jobs[i].cmd_line);
                jobs[i].in_use = 0;
            }
        }

        printf("%s:%s$ ", pw->pw_name, cwd);
        fflush(stdout);

        if (fgets(line, LINE_BUFFER_SIZE, stdin) == NULL)
            break;

        if (write(history_fd, line, strlen(line)) < 0) {
            perror("write history failed");
            break;
        }

        char normalized_line[LINE_BUFFER_SIZE * 2];
        normalize_redirects(line, normalized_line, sizeof(normalized_line));

        static struct Command pipeline[MAX_PIPELINE_STAGES];
        char pipe_delim[] = "|";
        int num_cmds = 0;
        char* pipe_saveptr = NULL;
        char* stage = strtok_r(normalized_line, pipe_delim, &pipe_saveptr);
        while (stage != NULL && num_cmds < MAX_PIPELINE_STAGES - 1) {
            if (parse_command(stage, &pipeline[num_cmds++])) {
                fprintf(stderr, "Error parsing command: %s\n", stage);
                num_cmds = 0;
                break;
            }
            stage = strtok_r(NULL, pipe_delim, &pipe_saveptr);
        }

        if (num_cmds == MAX_PIPELINE_STAGES - 1 && stage != NULL) {
            fprintf(stderr, "Error: Too many commands in pipeline (max %d)\n", MAX_PIPELINE_STAGES);
            continue;
        }

        if (num_cmds == 0 || pipeline[0].argv[0] == NULL)
            continue;

        int empty_stage = 0;
        for (int i = 0; i < num_cmds; i++) {
            if (pipeline[i].argv[0] == NULL) {
                fprintf(stderr, "Error: empty command in pipeline\n");
                empty_stage = 1;
                break;
            }
        }
        if (empty_stage)
            continue;

        builtin_result_t builtin_result = run_builtin(pipeline, history_path, pw);
        if (builtin_result == BUILTIN_EXIT)
            break;
        if (builtin_result == BUILTIN_HANDLED)
            continue;

        connect_pipeline(pipeline, num_cmds);
        execute_pipeline(pipeline, num_cmds, line);
    }
    return 0;
}
