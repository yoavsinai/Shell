#include "builtins/history.h"
#include "core/command.h"
#include "core/exit_status.h"
#include <errno.h>

builtin_result_t builtin_history(const char* history_path)
{
    FILE* history_file = fopen(history_path, "r");
    if (history_file == NULL) {
        last_exit_status = errno;
        perror("fopen history file failed");
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
