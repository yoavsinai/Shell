# CLAUDE.md

Guidance for Claude Code (or any future contributor) working in this repository.

## What this is

A small POSIX shell implemented in C: a REPL that supports piping (`|`), I/O
redirection (`<`, `>`, `>>`), command chaining (`&&`, `||`, `;`), quoted
arguments, `$VAR`/`$?`/`~` expansion, background jobs (`&`) with job control,
and the builtins `cd`, `!exit`, `history`, `jobs`, `fg`, `bg`, `kill`, and
`export`.

## Build & run

```sh
make          # builds build/shell
make run      # builds and launches it
make clean    # removes build/
```

No other build system is used. `CFLAGS` sets `-std=c11 -D_POSIX_C_SOURCE=200809L`
because POSIX functions used here (`strtok_r`, `getcwd`, `fork`, `execvp`, ...)
aren't visible under strict C11 without that feature-test macro. The Makefile
finds sources with `find src -name '*.c'` and mirrors `src/<subdir>/foo.c`
into `build/<subdir>/foo.o`, so a new subdirectory under `src/` needs no
Makefile changes.

## Layout

Headers live under `include/`, mirrored 1:1 by implementation files under
`src/`, grouped into folders by responsibility:

```
core/       command.h            struct Command + shared size constants
            exit_status.h        last_exit_status, read by "$?" expansion and
                                  written by every builtin/pipeline/parse
                                  failure path

parsing/    lexer.h              line preprocessing (redirect spacing, quote
                                  masking/unquoting)
            parse_command.h      tokens -> struct Command; also does
                                  $VAR/$?/~ expansion

exec/       pipe_connect.h       wire pipe(2) between adjacent pipeline stages
            pipe_exec.h          fork/exec/setpgid each stage, foreground
                                  wait (capturing the real exit status) or
                                  background job registration

jobs/       job_table.h          the global jobs[] table: add_job() and its
                                  add_background_job()/add_stopped_job()
                                  wrappers, find_job(), wait_for_job()

system/     shell_init.h         one-time startup: signal handlers, user
                                  lookup, history file
            signal_handler.h     SIGINT (ignore in the shell) and SIGCHLD
                                  (reap background jobs, mark them done)

line/       chain.h              splits a line on &&/||/; into
                                  chain_segment_t entries (pure text
                                  splitting, no execution)
            segment_scheduler.h  runs a line's chain_segment_t entries in
                                  order, short-circuiting &&/|| on
                                  last_exit_status
            pipeline_dispatch.h  parses one segment's '|' pipeline and
                                  dispatches it to a builtin or to
                                  pipe_connect+pipe_exec

builtins/   builtins.h           run_builtin(): looks up pipeline[0].argv[0]
                                  against every builtin below
            cd.h, history.h, jobs.h, fg.h, bg.h, kill.h, export.h
                                 one file per builtin command
            job_id.h             shared parse_job_id() helper for fg/bg/kill

main.c                            REPL loop: prompt, read, normalize, hand
                                   the line to segment_scheduler
tests/                             sample input files used for manual testing
```

Each `.c` file pairs with exactly one `.h` of the same name and path (except
`main.c`), which declares only what that file exports. `core/command.h` is
the one header nearly every module includes, since `struct Command` is the
shared currency between them.

There used to be two different `jobs.h` files (the job table and the `jobs`
builtin) — if you're tempted to add another same-named file in a different
folder, rename one of them instead; it's exactly this kind of ambiguity that
made the codebase hard to navigate before the `jobs/job_table.h` rename.

### How a line turns into execution

`main.c` calls `run_chain_segments()` (in `line/segment_scheduler.c`), which:

1. Calls `split_chain()` (in `line/chain.c`) to break the line on `&&`/`||`/
   `;` into `chain_segment_t` entries, each tagged with the operator that
   preceded it.
2. Walks those entries in order, and for each one gated by `&&`/`||`, checks
   `last_exit_status` from the previous entry that actually ran to decide
   whether to run this one; `;` always resets that gate.
3. For each entry that should run, calls `dispatch_pipeline()` (in
   `line/pipeline_dispatch.c`), which splits the segment on `|`, parses each
   stage with `parse_command()`, and either dispatches to `run_builtin()` or
   wires up (`connect_pipeline()`) and forks (`execute_pipeline()`) the
   pipeline.

### Why the pipeline is built in two passes

`dispatch_pipeline()` calls `connect_pipeline()` then `execute_pipeline()` as
separate steps. Pipes must be created and wired into every stage's
`in_fd`/`out_fd` *before* any process forks, otherwise a child wouldn't
inherit the right file descriptors. Don't collapse these into one pass
without preserving that ordering.

### Why `main.c` builds a `normalized_line` before tokenizing

`normalize_redirects()` inserts spaces around `<`, `>`, `>>` so that a plain
whitespace split treats them as standalone tokens even when the user wrote
`cat<file` with no spaces. This has to happen on the *whole* line before
splitting on chain operators or `|`, not inside `parse_command`, so pointers
into the buffer stay valid for the rest of the loop iteration (a local
buffer inside `parse_command` would go out of scope while `args[]` still
pointed into it).

### Quoting

Quote handling is a two-step trick, not a full lexer:

1. `mask_quoted_whitespace()` walks the line once and replaces whitespace
   *inside* `'...'`/`"..."` spans with a placeholder byte (`\x01`), so
   `strtok_r`'s ordinary whitespace split treats the quoted phrase as one
   token.
2. `unquote_token()` then strips the quote characters from each token and
   turns the placeholder back into real spaces.

This only handles simple, non-nested, non-escaped quoting — good enough for
`grep -v "some phrase"`, not a full shell-grammar quote parser. It also means
`split_chain()` and the `|` pipeline split don't track quoting either, so an
operator sequence inside quotes is still treated as a split point.

### `$?` / `last_exit_status`

`core/exit_status.h` exposes a single global, `last_exit_status`, read by
`parse_command()`'s `$?` expansion. Every place that can fail writes a real
error code into it rather than a hand-picked constant: `errno` from a failed
syscall (`chdir`, `open`, `setenv`, `fork`, `kill`, ...), `EINVAL`/`ESRCH`
for usage errors with no underlying syscall, `E2BIG` for "too many commands
in a pipeline", and the real `WEXITSTATUS`/`128+signal` for a completed
foreground pipeline or `fg`'d job. Keep new failure paths consistent with
this — don't introduce another bare `1`/`0` "did it work" flag.

## Known limitations (intentionally not "fixed" yet — ask before changing)

- `parse_command()`'s `arg_count` has no bound check against `MAX_ARGS` — a
  command with more than `MAX_ARGS - 1` arguments overflows `cmd->argv[]`.
- No escaping (`\"`, `\\`) inside or outside quotes.
- No globbing, subshells, or command substitution.
- `$VAR` expansion only handles a single `$NAME` per token (no `${...}`, no
  expansion inside quotes); `~user` (another user's home directory) isn't
  supported, only bare `~`/`~/...`.
- A builtin at the head of a `|` pipeline (e.g. `history | head -3`) runs
  standalone and ignores the rest of the pipeline — `dispatch_pipeline()`
  checks for a builtin before ever wiring up the pipe.
- `fg`'s `$?` is accurate for a job that runs to completion (via
  `wait_for_job()`), but a job that stops again reports the generic
  `128+SIGTSTP` rather than tracking per-process state further.
- Builtins are limited to `cd`, `!exit`, `history`, `jobs`, `fg`, `bg`,
  `kill`, and `export`; everything else goes through `execvp`.

## Conventions

- One responsibility per module, grouped by folder (see Layout above): line
  preprocessing (`parsing/lexer`), turning tokens into a `Command`
  (`parsing/parse_command`), wiring pipes (`exec/pipe_connect`),
  forking/exec'ing a pipeline (`exec/pipe_exec`), the background job table
  (`jobs/job_table`), signal handling (`system/signal_handler`), builtin
  commands (`builtins/`), chaining and dispatch (`line/`), and one-time
  startup (`system/shell_init`) are kept separate — don't fold unrelated
  concerns back into `main.c` or into each other. `main.c` should stay a
  thin REPL loop that wires the other modules together.
- Constants live in `core/command.h`: `MAX_ARGS` (args per single command,
  including the NULL terminator), `MAX_PIPELINE_STAGES` (commands chained
  with `|`), and `MAX_CHAIN_SEGMENTS` (segments chained with `&&`/`||`/`;`)
  are distinct — don't conflate them.
- No comments that restate what a line of code already says; comments here
  are reserved for non-obvious constraints (see above).
