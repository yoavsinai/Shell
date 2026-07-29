#ifndef BUILTINS_H
#define BUILTINS_H

#include "command.h"
#include <pwd.h>

typedef enum {
    BUILTIN_NOT_MATCHED, // pipeline[0].argv[0] isn't a builtin; caller should exec it
    BUILTIN_HANDLED, // builtin ran; caller should read the next line
    BUILTIN_EXIT // "!exit" was entered; caller should end the REPL
} builtin_result_t;

// Checks pipeline[0].argv[0] against the builtins (cd, !exit, history, jobs)
// and runs it if matched. history_path is the file cd/execvp'd commands
// never touch directly, only "history" reads it.
builtin_result_t run_builtin(struct Command* pipeline, const char* history_path, const struct passwd* pw);

#endif
