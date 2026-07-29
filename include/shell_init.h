#ifndef SHELL_INIT_H
#define SHELL_INIT_H

#include "command.h"
#include <pwd.h>

// One-time shell startup: installs SIGINT/SIGCHLD/SIGTTOU/SIGTTIN handlers,
// looks up the current user, and opens (creating if needed) the history
// file at history_path. Returns the user's passwd entry, or NULL if setup
// failed (already reported to stderr via perror). On success, *history_fd
// is left open for appending.
struct passwd* shell_init(char history_path[LINE_BUFFER_SIZE], int* history_fd);

#endif
