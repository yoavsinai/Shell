#ifndef BUILTINS_CD_H
#define BUILTINS_CD_H

#include "builtins/builtins.h"
#include "core/command.h"
#include <pwd.h>

builtin_result_t builtin_cd(struct Command* pipeline, const struct passwd* pw);

#endif
