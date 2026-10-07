#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char *end;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s n (0..10)\n", argv[0]);
        return EXIT_FAILURE;
    }
    errno = 0;
    long n = strtol(argv[1], &end, 10);
    if (errno || end == argv[1] || *end != '\0' || n < 0 || n > 10) {
        fputs("n must be an integer from 0 to 10\n", stderr);
        return EXIT_FAILURE;
    }
    int result = EXIT_SUCCESS;
    for (long i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            result = EXIT_FAILURE;
            break;
        }
        printf("Iteration %ld: PID=%ld PPID=%ld\n", i + 1,
               (long)getpid(), (long)getppid());
        fflush(stdout); /* Do not copy buffered output into the next child. */
        unsigned remaining = 5;
        while (remaining != 0)
            remaining = sleep(remaining);
        /* Both the parent and child continue the loop. */
    }
    for (;;) {
        int status;
        pid_t pid = wait(&status);
        if (pid > 0) {
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                result = EXIT_FAILURE;
        } else if (errno != EINTR) {
            if (errno != ECHILD) {
                perror("wait");
                result = EXIT_FAILURE;
            }
            break;
        }
    }
    return result;
}
