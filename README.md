# Shell

A small Unix shell written in C, built as a learning project covering the
core mechanics of a shell: parsing, I/O redirection, piping, and process
management.

## Features

- Command execution via `fork` + `execvp`
- Piping: `cmd1 | cmd2 | cmd3`
- I/O redirection: `<`, `>`, `>>` (works with or without surrounding spaces,
  e.g. both `cat < file` and `cat<file`)
- Quoted arguments: `grep -v "some phrase" < file`
- Builtins: `cd [dir]` and `!exit`

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
yourname:/tmp$ !exit
```

## Clean

```sh
make clean
```

Removes the `build/` directory.

## Project layout

```
include/    headers, one per module (command, lexer, parse_command, executor)
src/        implementation files
tests/      sample input files for manual testing
Makefile
```

See [CLAUDE.md](CLAUDE.md) for a deeper look at the internals and known
limitations.
