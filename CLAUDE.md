# CLAUDE.md

Guidance for Claude Code (or any future contributor) working in this repository.

## What this is

A small POSIX shell implemented in C: a REPL that supports piping (`|`), I/O
redirection (`<`, `>`, `>>`), quoted arguments, and the builtins `cd` and
`!exit`.

## Build & run

```sh
make          # builds build/shell
make run      # builds and launches it
make clean    # removes build/
```

No other build system is used. `CFLAGS` sets `-std=c11 -D_POSIX_C_SOURCE=200809L`
because POSIX functions used here (`strtok_r`, `getcwd`, `fork`, `execvp`, ...)
aren't visible under strict C11 without that feature-test macro.

## Layout

```
include/command.h        struct Command + shared size constants
include/lexer.h + src/lexer.c              line preprocessing (redirect
                                            spacing, quote masking/unquoting)
include/parse_command.h + src/parse_command.c   tokens -> struct Command
include/executor.h + src/executor.c        wire up pipes, fork/exec, wait
src/main.c                                 REPL loop, builtins, wiring
tests/                                     sample input files used for manual testing
```

Each `.c` file pairs with exactly one `.h` of the same name (except
`main.c`), which declares only what that file exports. `command.h` is the
one header every module includes, since `struct Command` is the shared
currency between them.

### Why the pipeline is built in two passes

`main.c` calls `connect_pipeline()` then `execute_pipeline()` as separate
steps. Pipes must be created and wired into every stage's `in_fd`/`out_fd`
*before* any process forks, otherwise a child wouldn't inherit the right
file descriptors. Don't collapse these into one pass without preserving that
ordering.

### Why `main.c` builds a `normalized_line` before tokenizing

`normalize_redirects()` inserts spaces around `<`, `>`, `>>` so that a plain
whitespace split treats them as standalone tokens even when the user wrote
`cat<file` with no spaces. This has to happen on the *whole* line before
splitting on `|`, not inside `parse_command`, so pointers into the buffer
stay valid for the rest of the loop iteration (a local buffer inside
`parse_command` would go out of scope while `args[]` still pointed into it).

### Quoting

Quote handling is a two-step trick, not a full lexer:

1. `mask_quoted_whitespace()` walks the line once and replaces whitespace
   *inside* `'...'`/`"..."` spans with a placeholder byte (`\x01`), so
   `strtok_r`'s ordinary whitespace split treats the quoted phrase as one
   token.
2. `unquote_token()` then strips the quote characters from each token and
   turns the placeholder back into real spaces.

This only handles simple, non-nested, non-escaped quoting — good enough for
`grep -v "some phrase"`, not a full shell-grammar quote parser.

## Known limitations (intentionally not "fixed" yet — ask before changing)

- `parse_command()`'s `arg_count` has no bound check against `MAX_ARGS` — a
  command with more than 9 arguments overflows `cmd->args[]`.
- No escaping (`\"`, `\\`) inside or outside quotes.
- No `&&`, `;`, background jobs (`&`), globbing, or variable expansion.
- Builtins are limited to `cd` and `!exit`; everything else goes through
  `execvp`.

## Conventions

- One responsibility per module: line preprocessing (`lexer`), turning
  tokens into a `Command` (`parse_command`), and running a pipeline
  (`executor`) are kept separate — don't fold unrelated concerns back into
  `main.c` or into each other.
- Constants live in `command.h`: `MAX_ARGS` (args per single command,
  including the NULL terminator) is distinct from `MAX_PIPELINE_STAGES`
  (commands chained with `|`) — don't conflate them.
- No comments that restate what a line of code already says; comments here
  are reserved for non-obvious constraints (see above).
