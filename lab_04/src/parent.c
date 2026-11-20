#include "../include/common.h"
#include "../include/parent.h"
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int read_file_name(char *buffer, size_t buffer_size) {
    printf("Введите имя входного файла: ");
    fflush(stdout);

    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        fprintf(stderr, "Не удалось прочитать имя файла.\n");
        return -1;
    }

    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else if (length == buffer_size - 1) {
        fprintf(stderr, "Слишком длинное имя файла.\n");
        return -1;
    }

    if (buffer[0] == '\0') {
        fprintf(stderr, "Имя файла не может быть пустым.\n");
        return -1;
    }
    return 0;
}

static int make_child_path(const char *program_name, char *buffer,
                           size_t buffer_size) {
    const char *last_slash = strrchr(program_name, '/');
    if (last_slash == NULL) {
        if (snprintf(buffer, buffer_size, "child") >= (int)buffer_size) {
            return -1;
        }
        return 0;
    }

    size_t directory_length = (size_t)(last_slash - program_name + 1);
    if (directory_length + strlen("child") + 1 > buffer_size) {
        return -1;
    }

    memcpy(buffer, program_name, directory_length);
    strcpy(buffer + directory_length, "child");
    return 0;
}

static int wait_for_message(sem_t *ready, pid_t child_pid,
                            int *child_was_reaped, int *child_status) {
    const struct timespec pause = {.tv_sec = 0, .tv_nsec = 10 * 1000 * 1000};

    while (1) {
        if (sem_trywait(ready) == 0) {
            return 0;
        }
        if (errno != EAGAIN && errno != EINTR) {
            perror("sem_trywait");
            return -1;
        }

        pid_t result = waitpid(child_pid, child_status, WNOHANG);
        if (result == child_pid) {
            *child_was_reaped = 1;
            fprintf(stderr, "Дочерний процесс завершился, не отправив сообщение.\n");
            return -1;
        }
        if (result == -1 && errno != EINTR) {
            perror("waitpid");
            return -1;
        }
        nanosleep(&pause, NULL);
    }
}

typedef struct {
    char mapping_path[64];
    char empty_name[64];
    char ready_name[64];
    int input_fd;
    int mapping_fd;
    SharedData *shared;
    sem_t *empty;
    sem_t *ready;
    pid_t child_pid;
    int child_was_reaped;
    int child_status;
} ParentResources;

static int cleanup_parent(ParentResources *resources, int result) {
    if (resources->child_pid > 0 && !resources->child_was_reaped) {
        if (result != EXIT_SUCCESS) {
            kill(resources->child_pid, SIGTERM);
        }

        while (waitpid(resources->child_pid, &resources->child_status, 0) == -1 &&
               errno == EINTR) {
        }

        if (result == EXIT_SUCCESS &&
            (!WIFEXITED(resources->child_status) ||
             WEXITSTATUS(resources->child_status) != 0)) {
            fprintf(stderr, "Дочерний процесс завершился с ошибкой.\n");
            result = EXIT_FAILURE;
        }
    }

    if (resources->ready != SEM_FAILED) {
        sem_close(resources->ready);
        sem_unlink(resources->ready_name);
    }
    if (resources->empty != SEM_FAILED) {
        sem_close(resources->empty);
        sem_unlink(resources->empty_name);
    }
    if (resources->shared != MAP_FAILED) {
        munmap(resources->shared, sizeof(SharedData));
    }
    if (resources->mapping_fd != -1) {
        close(resources->mapping_fd);
        unlink(resources->mapping_path);
    }
    if (resources->input_fd != -1) {
        close(resources->input_fd);
    }

    return result;
}

int run_parent(const char *program_name) {
    char input_path[4096];
    char child_path[4096];
    ParentResources resources = {
        .mapping_path = "/tmp/lab4_mapping_XXXXXX",
        .input_fd = -1,
        .mapping_fd = -1,
        .shared = MAP_FAILED,
        .empty = SEM_FAILED,
        .ready = SEM_FAILED,
        .child_pid = -1,
    };

    if (read_file_name(input_path, sizeof(input_path)) == -1) {
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    resources.input_fd = open(input_path, O_RDONLY);
    if (resources.input_fd == -1) {
        perror("Не удалось открыть входной файл");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    if (make_child_path(program_name, child_path, sizeof(child_path)) == -1) {
        fprintf(stderr, "Не удалось сформировать путь к программе child.\n");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    resources.mapping_fd = mkstemp(resources.mapping_path);
    if (resources.mapping_fd == -1) {
        perror("mkstemp");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }
    if (ftruncate(resources.mapping_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    resources.shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                            MAP_SHARED, resources.mapping_fd, 0);
    if (resources.shared == MAP_FAILED) {
        perror("mmap");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }
    memset(resources.shared, 0, sizeof(*resources.shared));

    snprintf(resources.empty_name, sizeof(resources.empty_name),
             "/lab4_empty_%ld", (long)getpid());
    snprintf(resources.ready_name, sizeof(resources.ready_name),
             "/lab4_ready_%ld", (long)getpid());
    sem_unlink(resources.empty_name);
    sem_unlink(resources.ready_name);

    resources.empty = sem_open(resources.empty_name, O_CREAT | O_EXCL, 0600, 1);
    if (resources.empty == SEM_FAILED) {
        perror("sem_open(empty)");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }
    resources.ready = sem_open(resources.ready_name, O_CREAT | O_EXCL, 0600, 0);
    if (resources.ready == SEM_FAILED) {
        perror("sem_open(ready)");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    resources.child_pid = fork();
    if (resources.child_pid == -1) {
        perror("fork");
        return cleanup_parent(&resources, EXIT_FAILURE);
    }

    if (resources.child_pid == 0) {
        char fd_text[32];

        if (dup2(resources.input_fd, STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(126);
        }
        close(resources.input_fd);

        snprintf(fd_text, sizeof(fd_text), "%d", resources.mapping_fd);
        execlp(child_path, "child", fd_text, resources.empty_name,
               resources.ready_name,
               (char *)NULL);
        perror("exec child");
        _exit(127);
    }

    close(resources.input_fd);
    resources.input_fd = -1;

    while (1) {
        if (wait_for_message(resources.ready, resources.child_pid,
                             &resources.child_was_reaped,
                             &resources.child_status) == -1) {
            return cleanup_parent(&resources, EXIT_FAILURE);
        }

        if (resources.shared->type == MESSAGE_RESULT) {
            printf("Строка %zu: сумма = %lld\n", resources.shared->line_number,
                   (long long)resources.shared->sum);
            if (sem_post(resources.empty) == -1) {
                perror("sem_post(empty)");
                return cleanup_parent(&resources, EXIT_FAILURE);
            }
        } else if (resources.shared->type == MESSAGE_DONE) {
            sem_post(resources.empty);
            return cleanup_parent(&resources, EXIT_SUCCESS);
        } else if (resources.shared->type == MESSAGE_ERROR) {
            fprintf(stderr, "Ошибка в дочернем процессе: %s\n",
                    resources.shared->error_text);
            sem_post(resources.empty);
            return cleanup_parent(&resources, EXIT_FAILURE);
        } else {
            fprintf(stderr, "Получен неизвестный тип сообщения.\n");
            sem_post(resources.empty);
            return cleanup_parent(&resources, EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv) {
    (void)argc;
    return run_parent(argv[0]);
}
