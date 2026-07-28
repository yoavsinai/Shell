#ifndef PARSE_COMMAND_H
#define PARSE_COMMAND_H

#include "command.h"

// Tokenizes one pipeline stage (already split off by '|') into cmd->argv,
// applying '<'/'>'/'>>' redirection into cmd->in_fd/out_fd as encountered.
// cmd_str is modified in place. Returns 0 on success, 1 on a parse/open
// error (already reported to stderr).
int parse_command(char* cmd_str, struct Command* cmd);

#endif
