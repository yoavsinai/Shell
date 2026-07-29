#include "builtins/history.h"
#include "command.h"
#include "exit_status.h"

builtin_result_t builtin_history(const char* history_path)
{
    FILE* history_file = fopen(history_path, "r");
    if (history_file == NULL) {
        perror("fopen history file failed");
        last_exit_status = 1;
        return BUILTIN_HANDLED;
    }
    char history_line[LINE_BUFFER_SIZE];
    int line_number = 1;
    while (fgets(history_line, sizeof(history_line), history_file)) {
        printf("%d: %s", line_number++, history_line);
    }
    fclose(history_file);
    last_exit_status = 0;
    return BUILTIN_HANDLED;
}
