#include "system/shell_init.h"
#include "system/signal_handler.h"

struct passwd* shell_init(char history_path[LINE_BUFFER_SIZE], int* history_fd)
{
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw == NULL) {
        perror("getpwuid failed");
        return NULL;
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    struct sigaction act = { 0 };
    act.sa_handler = &sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction failed");
        return NULL;
    }

    act.sa_handler = &sigchld_handler;

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        perror("sigaction failed");
        return NULL;
    }

    snprintf(history_path, LINE_BUFFER_SIZE, "%s/.my_shell_history", getenv("HOME") ? getenv("HOME") : "/tmp/.my_shell_history");

    *history_fd = open(history_path, O_CREAT | O_APPEND | O_WRONLY, 0644);
    if (*history_fd < 0) {
        perror("open history file failed");
        return NULL;
    }

    return pw;
}
