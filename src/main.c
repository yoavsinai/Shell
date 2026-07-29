#include "command.h"
#include "executor.h"
#include "lexer.h"
#include "parse_command.h"
#include "signal_handler.h"

int main()
{
    char line[LINE_BUFFER_SIZE];
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw == NULL) {
        perror("getpwuid failed");
        return 0;
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    struct sigaction act = { 0 };
    act.sa_handler = &sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction failed");
        return 0;
    }

    act.sa_handler = &sigchld_handler;

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        perror("sigaction failed");
        return 0;
    }

    char history_path[LINE_BUFFER_SIZE];
    sprintf(history_path, "%s/.my_shell_history", getenv("HOME") ? getenv("HOME") : "/tmp/.my_shell_history");

    int history_fd = open(history_path, O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (history_fd < 0) {
        perror("open history file failed");
        return 0;
    }

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

        struct Command pipeline[MAX_PIPELINE_STAGES];
        char pipe_delim[] = "|";
        int num_cmds = 0;
        char* pipe_saveptr = NULL;
        char* stage = strtok_r(normalized_line, pipe_delim, &pipe_saveptr);
        while (stage != NULL && num_cmds < MAX_PIPELINE_STAGES - 1) {
            if (parse_command(stage, &pipeline[num_cmds++])) {
                fprintf(stderr, "Error parsing command: %s\n", stage);
                break;
            }
            stage = strtok_r(NULL, pipe_delim, &pipe_saveptr);
        }

        if (num_cmds == MAX_PIPELINE_STAGES - 1 && stage != NULL) {
            fprintf(stderr, "Error: Too many commands in pipeline (max %d)\n", MAX_PIPELINE_STAGES);
            continue;
        }

        if (pipeline[0].argv[0] == NULL)
            continue;

        if (strcmp(pipeline[0].argv[0], "!exit") == 0)
            break;
        if (strcmp(pipeline[0].argv[0], "cd") == 0) {
            const char* target_dir = pipeline[0].argv[1] ? pipeline[0].argv[1] : pw->pw_dir;
            if (chdir(target_dir) < 0) {
                perror("chdir failed");
            }
            continue;
        }
        if (strcmp(pipeline[0].argv[0], "history") == 0) {
            FILE* history_file = fopen(history_path, "r");
            if (history_file == NULL) {
                perror("fopen history file failed");
                continue;
            }
            char history_line[LINE_BUFFER_SIZE];
            int line_number = 1;
            while (fgets(history_line, sizeof(char) * LINE_BUFFER_SIZE, history_file)) {
                printf("%d: %s", line_number++, history_line);
            }
            fclose(history_file);
            continue;
        }
        if (strcmp(pipeline[0].argv[0], "jobs") == 0) {
            for (int i = 0; i < MAX_JOBS; i++) {
                if (jobs[i].in_use) {
                    const char* state_str = (jobs[i].state == JOB_RUNNING) ? "Running" : (jobs[i].state == JOB_STOPPED) ? "Stopped"
                                                                                                                        : "Done";
                    printf("[%d] %s\t%s\n", jobs[i].job_id, state_str, jobs[i].cmd_line);
                }
            }
            continue;
        }

        connect_pipeline(pipeline, num_cmds);
        execute_pipeline(pipeline, num_cmds, line);
    }
    return 0;
}
