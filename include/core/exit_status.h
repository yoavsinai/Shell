#ifndef EXIT_STATUS_H
#define EXIT_STATUS_H

// Exit status of the most recently completed foreground command (pipeline
// stage or builtin). Read by parse_command.c to expand "$?" and written by
// execute_pipeline() and the builtins after they run.
extern int last_exit_status;

#endif
