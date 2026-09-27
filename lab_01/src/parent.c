#include <stdio.h>
#include "../include/parent.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int parent(void) {
    char *string = NULL;
    size_t len = 0;
    const ssize_t count = getline(&string, &len, stdin);
    if (count == -1) {
        free(string);
        return -1;
    }
    if (string[count - 1] == '\n') string[count - 1] = '\0';
    int desc = open(string, O_RDONLY);
    if (desc == -1) {
        perror("open");
        free(string);
        return -1;
    }
    int arr[2];
    const int p = pipe(arr);
    if (p == -1) {
        perror("pipe");
        free(string);
        close(desc);
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        free(string);
        close(arr[0]);
        close(arr[1]);
        close(desc);
        return -1;
    }
    if (pid == 0) {
        // CHILD
        close(arr[0]);
        const int change = dup2(desc, STDIN_FILENO);
        if (change == -1) {
            perror("dup2 stdin");
            free(string);
            close(arr[1]);
            close(desc);
            return -1;
        }
        close(desc);
        const int change2 = dup2(arr[1], STDOUT_FILENO);
        if (change2 == -1) {
            perror("dup2 stdout");
            free(string);
            close(arr[1]);
            return -1;
        }
        close(arr[1]);
        execl("./child", "child", NULL);
        perror("execl");
        free(string);
        return -1;
    } else {
        // PARENT
        close(arr[1]);
        close(desc);
        char buffer[256];
        ssize_t bytes;

        while ((bytes = read(arr[0], buffer, sizeof(buffer))) > 0) {
            ssize_t written = 0;
            while (written < bytes) {
                const ssize_t result = write(STDOUT_FILENO, buffer + written, bytes - written);
                if (result == -1) {
                    perror("write");
                    free(string);
                    close(arr[0]);
                    return -1;
                }
                written += result;
            }
        }
        if (bytes == -1) {
            perror("read");
            free(string);
            close(arr[0]);
            return -1;
        }
        close(arr[0]);
        free(string);

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return -1;
        }

        if (!WIFEXITED(status)) {
            fprintf(stderr, "Child terminated abnormally\n");
            return -1;
        }
        if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Child exited with code %d\n", WEXITSTATUS(status));
            return -1;
        }
    }
    return 0;
}

int main(void) {
    return parent();
}
