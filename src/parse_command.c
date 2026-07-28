#include "parse_command.h"
#include "lexer.h"

int parse_command(char* cmd_str, struct Command* cmd)
{
    cmd->in_fd = STDIN_FILENO;
    cmd->out_fd = STDOUT_FILENO;

    mask_quoted_whitespace(cmd_str);

    int argv_count = 0;
    char* saveptr;
    char delim[] = " \t\r\n";
    char* token = strtok_r(cmd_str, delim, &saveptr);

    while (token != NULL) {
        unquote_token(token);

        if (token[0] == '$') {
            char* env_var = getenv(token + 1);
            if (env_var != NULL) {
                token = env_var;
            } else {
                token = "";
            }
        }

        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, delim, &saveptr);
            if (token == NULL) {
                fprintf(stderr, "Error: No input file specified for redirection\n");
                return 1;
            }
            unquote_token(token);
            cmd->in_fd = open(token, O_RDONLY);
            if (cmd->in_fd < 0) {
                perror("open input failed");
                return 1;
            }

        } else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, delim, &saveptr);
            if (token == NULL) {
                fprintf(stderr, "Error: No output file specified for redirection\n");
                return 1;
            }
            unquote_token(token);
            cmd->out_fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (cmd->out_fd < 0) {
                perror("open output failed");
                return 1;
            }

        } else if (strcmp(token, ">>") == 0) {
            token = strtok_r(NULL, delim, &saveptr);
            if (token == NULL) {
                fprintf(stderr, "Error: No output file specified for append redirection\n");
                return 1;
            }
            unquote_token(token);
            cmd->out_fd = open(token, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (cmd->out_fd < 0) {
                perror("open append failed");
                return 1;
            }

        } else {
            cmd->argv[argv_count++] = token;
        }
        token = strtok_r(NULL, delim, &saveptr);
    }

    cmd->argv[argv_count] = NULL;
    cmd->argc = argv_count;
    return 0;
}
