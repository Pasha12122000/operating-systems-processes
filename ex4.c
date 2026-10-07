#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void reap_children(int signal_number)
{
    (void)signal_number;
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
    errno = saved_errno;
}

int main(void)
{
    struct sigaction action = {0};
    action.sa_handler = reap_children;
    action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGCHLD, &action, NULL) < 0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;
    int result = EXIT_SUCCESS;
    for (;;) {
        if (isatty(STDIN_FILENO)) {
            fputs("mini-shell> ", stdout);
            fflush(stdout);
        }
        if (getline(&line, &capacity, stdin) < 0) {
            if (ferror(stdin)) {
                perror("getline");
                result = EXIT_FAILURE;
            }
            break;
        }
        char *args[128];
        size_t count = 0;
        char *save = NULL;
        char *token = strtok_r(line, " \t\r\n", &save);
        while (token != NULL && count < 127) {
            args[count++] = token;
            token = strtok_r(NULL, " \t\r\n", &save);
        }
        if (token != NULL) {
            fputs("Too many arguments (maximum 127 tokens)\n", stderr);
            continue;
        }
        if (count == 0)
            continue;
        args[count] = NULL;
        if (strcmp(args[0], "exit") == 0 && count == 1)
            break;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            execvp(args[0], args); /* Find the executable using PATH. */
            perror(args[0]);
            _exit(127);
        }
        printf("Started PID=%ld\n", (long)pid);
        fflush(stdout);
        /* No blocking wait: immediately accept the next command. */
    }
    free(line);
    return result;
}
