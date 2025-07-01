// problema1.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("mi_shell> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0; // quitar salto de línea

        if (strcmp(input, "exit") == 0) break;

        // separar argumentos
        int argn = 0;
        char *token = strtok(input, " ");
        while (token && argn < MAX_ARGS - 1) {
            args[argn++] = token;
            token = strtok(NULL, " ");
        }
        args[argn] = NULL;

        int fd[2];
        pipe(fd);

        pid_t pid = fork();
        if (pid == 0) {
            // hijo
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO); // redirige salida estándar
            close(fd[1]);

            execvp(args[0], args);
            perror("execvp"); // solo si falla
            exit(1);
        } else if (pid > 0) {
            // padre
            close(fd[1]);
            char buffer[1024];
            ssize_t n;

            while ((n = read(fd[0], buffer, sizeof(buffer)-1)) > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);
            }
            close(fd[0]);
            waitpid(pid, NULL, 0);
        } else {
            perror("fork");
        }
    }

    return 0;
}
