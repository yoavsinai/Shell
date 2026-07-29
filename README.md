# Shell

A small Unix shell written in C, built as a learning project covering the
core mechanics of a shell: parsing, I/O redirection, piping, job control,
and process management.

## Features

- Command execution via `fork` + `execvp`
- Piping: `cmd1 | cmd2 | cmd3`
- I/O redirection: `<`, `>`, `>>` (works with or without surrounding spaces,
  e.g. both `cat < file` and `cat<file`)
- Command chaining: `cmd1 && cmd2`, `cmd1 || cmd2`, `cmd1 ; cmd2`
- Quoted arguments: `grep -v "some phrase" < file`
- `$VAR` expansion (`echo $HOME`), `$?` for the last exit status, and `~`/
  `~/...` expansion to `$HOME`
- Background jobs: `sleep 10 &`, listed with `jobs`, reaped automatically
  when they finish; `fg`/`bg`/`kill` for job control
- Builtins: `cd [dir]`, `!exit`, `history`, `jobs`, `fg`, `bg`, `kill`, and
  `export NAME=VALUE`

## Build

Requires `gcc` and `make` on a POSIX system (Linux/macOS).

```sh
make
```

This produces `build/shell`.

## Run

```sh
make run
# or
./build/shell
```

You'll get a prompt like:

```
yourname:/current/dir$
```

Example session:

```
yourname:~$ echo "hello   world"
hello   world
yourname:~$ sort < tests/input | uniq
15
yourname:~$ echo hi >> out.txt
yourname:~$ cd /tmp
yourname:/tmp$ false || echo "that failed, exit code $?"
that failed, exit code 1
yourname:/tmp$ sleep 30 &
[1] 12345
yourname:/tmp$ jobs
[1] Running	sleep 30 &
yourname:/tmp$ kill -9 1
yourname:/tmp$ !exit
```

## Clean

```sh
make clean
```

Removes the `build/` directory.

## Project layout

```
include/, src/
  core/        struct Command, size constants, and the $? state
  parsing/     line preprocessing + tokenizing into struct Command
  exec/        wiring pipes and forking/exec'ing a pipeline
  jobs/        the background job table
  system/      one-time startup and signal handling
  line/        &&/||/; chaining, and dispatching a pipeline to a builtin or exec
  builtins/    cd, history, jobs, fg, bg, kill, export (one file per command)
  main.c       the REPL loop

tests/      sample input files for manual testing
Makefile
```

See [CLAUDE.md](CLAUDE.md) for a deeper look at the internals and known
limitations.
