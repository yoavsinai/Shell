#include "signal_handler.h"

void sigint_handler(int signo) {
    return;
}

void sigchld_handler(int signo) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // keep reaping until none are left
    }
    errno = saved_errno;

}