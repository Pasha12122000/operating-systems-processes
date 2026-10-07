#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void print_info(const char *name, clock_t start)
{
    clock_t end = clock();
    printf("%s: PID=%ld PPID=%ld CPU time=%.3f ms\n", name,
           (long)getpid(), (long)getppid(),
           1000.0 * (double)(end - start) / CLOCKS_PER_SEC);
}

int main(void)
{
    pid_t child1 = fork();
    clock_t start = clock(); /* First instruction after the first fork. */
    if (child1 < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child1 == 0) {
        print_info("Child 1", start);
        return EXIT_SUCCESS; /* This child never reaches the second fork. */
    }

    pid_t child2 = fork();
    clock_t child2_start = clock(); /* First instruction after second fork. */
    int result = EXIT_SUCCESS;
    if (child2 < 0) {
        perror("fork");
        result = EXIT_FAILURE;
    } else if (child2 == 0) {
        print_info("Child 2", child2_start);
        return EXIT_SUCCESS;
    }

    /* Keep the parent alive until both children finish. */
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
    print_info("Parent", start);
    return result;
}
