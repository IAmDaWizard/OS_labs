#include "child.h"
#include "common.h"
#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int send_message(SharedData *shared, sem_t *empty, sem_t *ready,
                        MessageType type, size_t line_number, int64_t sum,
                        const char *error) {
    while (sem_wait(empty) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    shared->type = type;
    shared->line_number = line_number;
    shared->sum = sum;
    snprintf(shared->error_text, sizeof(shared->error_text), "%s",
             error == NULL ? "" : error);

    return sem_post(ready);
}

static int sum_line(char *line, int64_t *sum) {
    char *number = strtok(line, " \t\r\n");
    *sum = 0;

    if (number == NULL) {
        return -1;
    }
    while (number != NULL) {
        errno = 0;
        char *end;
        long value = strtol(number, &end, 10);
        if (*end != '\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX) {
            return -1;
        }

        *sum += value;
        number = strtok(NULL, " \t\r\n");
    }
    return 0;
}

static int cleanup_child(int mapping_fd, SharedData *shared, sem_t *empty,
                         sem_t *ready, char *line, int result) {
    free(line);
    if (ready != SEM_FAILED) {
        sem_close(ready);
    }
    if (empty != SEM_FAILED) {
        sem_close(empty);
    }
    if (shared != MAP_FAILED) {
        munmap(shared, sizeof(SharedData));
    }
    close(mapping_fd);
    return result;
}

int run_child(int mapping_fd, const char *empty_name, const char *ready_name) {
    SharedData *shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                              MAP_SHARED, mapping_fd, 0);
    sem_t *empty = SEM_FAILED;
    sem_t *ready = SEM_FAILED;
    char *line = NULL;
    size_t capacity = 0;
    size_t line_number = 0;

    if (shared == MAP_FAILED) {
        perror("child: mmap");
        return cleanup_child(mapping_fd, shared, empty, ready, line,
                             EXIT_FAILURE);
    }

    empty = sem_open(empty_name, 0);
    ready = sem_open(ready_name, 0);
    if (empty == SEM_FAILED || ready == SEM_FAILED) {
        perror("child: sem_open");
        return cleanup_child(mapping_fd, shared, empty, ready, line,
                             EXIT_FAILURE);
    }

    while (getline(&line, &capacity, stdin) != -1) {
        int64_t sum;
        ++line_number;

        if (sum_line(line, &sum) == -1) {
            char error[256];
            snprintf(error, sizeof(error), "некорректные данные в строке %zu",
                     line_number);
            send_message(shared, empty, ready, MESSAGE_ERROR, line_number, 0,
                         error);
            return cleanup_child(mapping_fd, shared, empty, ready, line,
                                 EXIT_FAILURE);
        }

        if (send_message(shared, empty, ready, MESSAGE_RESULT, line_number, sum,
                         NULL) == -1) {
            perror("child: send_message");
            return cleanup_child(mapping_fd, shared, empty, ready, line,
                                 EXIT_FAILURE);
        }
    }

    if (ferror(stdin)) {
        send_message(shared, empty, ready, MESSAGE_ERROR, line_number, 0,
                     "ошибка чтения файла");
        return cleanup_child(mapping_fd, shared, empty, ready, line,
                             EXIT_FAILURE);
    }

    int result = send_message(shared, empty, ready, MESSAGE_DONE, line_number,
                              0, NULL) == 0
                     ? EXIT_SUCCESS
                     : EXIT_FAILURE;
    return cleanup_child(mapping_fd, shared, empty, ready, line, result);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Использование: child <fd> <empty> <ready>\n");
        return EXIT_FAILURE;
    }

    char *end;
    long mapping_fd = strtol(argv[1], &end, 10);
    if (*end != '\0' || mapping_fd < 0 || mapping_fd > INT_MAX) {
        fprintf(stderr, "child: неверный файловый дескриптор\n");
        return EXIT_FAILURE;
    }

    return run_child((int)mapping_fd, argv[2], argv[3]);
}
