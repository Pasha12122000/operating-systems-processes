# Operating Systems: processes and a simple shell

Solutions to the three screenshot exercises. The requested filenames are
`ex1.c`, `ex3.c`, and `ex4.c` (the slide exercise numbers are 1, 2, and 3).
Requires Linux or another POSIX system, and a C compiler. Windows users can use WSL.

## Compile and run

```sh
cc -std=c11 -Wall -Wextra -Wpedantic ex1.c -o ex1
cc -std=c11 -Wall -Wextra -Wpedantic ex3.c -o ex3
cc -std=c11 -Wall -Wextra -Wpedantic ex4.c -o ex4
./ex1
./ex4
```

## ex1: two sibling child processes

Only the original parent performs both forks. The first child prints its
information and exits before the second fork. Both children's PPIDs therefore
equal the original parent's PID. The parent waits for its children and prints
its own information. Output order between children can vary.

Each process measures CPU time using `clock()`, starting at the first instruction
after the fork that created it; the original parent starts immediately after the
first fork. Time is converted to milliseconds with `CLOCKS_PER_SEC`. This is CPU
time, not elapsed wall time, so waiting contributes little and values can round
to zero for this small program.

## ex3: repeated fork experiment

`n` is a command-line argument. Every process continues the loop and sleeps for
five seconds after each successful fork. Values 0..10 are accepted to limit
accidental exponential process growth. Each process reaps its direct children
after the loop.

On Linux, run in the background and inspect the tree repeatedly during execution
(install `pstree`, usually from the `psmisc` package, if needed):

```sh
./ex3 3 > ex3-n3.log &
root_pid=$!
pstree -p "$root_pid"
sleep 6
pstree -p "$root_pid"
sleep 5
pstree -p "$root_pid"
wait "$root_pid"
cat ex3-n3.log

./ex3 5 > ex3-n5.log &
root_pid=$!
pstree -p "$root_pid"
sleep 6
pstree -p "$root_pid"
sleep 5
pstree -p "$root_pid"
sleep 5
pstree -p "$root_pid"
sleep 5
pstree -p "$root_pid"
wait "$root_pid"
cat ex3-n5.log
```

Every iteration doubles the current number of processes: each existing process
creates one child, and both continue. Assuming all forks succeed:

| n | Process counts after successive iterations | Total including original | New children | Approximate duration |
|---|---|---|---|---|
| 3 | 2, 4, 8 | 8 | 7 | 15 seconds |
| 5 | 2, 4, 8, 16, 32 | 32 | 31 | 25 seconds |

The general formula is `2^n` total processes and `2^n - 1` newly created
processes. For n=3, the child counts per iteration are 1 + 2 + 4 = 7.
For n=5, they are 1 + 2 + 4 + 8 + 16 = 31. Adding two iterations multiplies
the total by four, from 8 to 32; it does not merely add two processes.

There are 14 iteration log lines for n=3 and 62 for n=5, since both parent and
child print on each iteration (2 + 4 + ... + 2^n). Count unique PIDs, or count
PIDs in the final iteration, to obtain the total. Output ordering and precise
timings depend on scheduling. Inspect before the last sleep ends; afterward the
processes exit and the tree disappears. `pstree` availability on macOS differs.

## ex4: simple background shell

Enter commands such as:

```text
pwd
ls
ls -l
echo hello world
sleep 5
exit
```

Each external command runs in a child created with `fork()`, and `execvp()`
replaces that child with the requested program. The shell immediately accepts
another command: all external commands run in the background, without an `&`
operator and without `system()`. `SIGCHLD` and nonblocking `waitpid()` reap
finished children to prevent zombies while the shell is running.

Arguments are separated by whitespace. This intentionally small shell does not
implement quoting, pipes, redirection, variable expansion, or job control.
`exit` (or EOF/Ctrl-D) closes the shell; already running commands may continue.
Background output can appear beside the prompt. Interactive programs such as
`top` may compete for terminal input; use a noninteractive mode when available.
