#include "core/command.h"
#include "jobs/job_table.h"
#include "parsing/lexer.h"
#include "line/segment_scheduler.h"
#include "system/shell_init.h"

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

        if (run_chain_segments(normalized_line, line, history_path, pw))
            break;
    }
    return 0;
}
