#include <stdio.h>
#include <pthread.h>
#include "../include/structs.h"
#include "../include/worker.h"
#include "../include/matrix_fill.h"
#include "../include/matrix_spawn.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if ((argc != 3 && argc != 4) || strcmp(argv[1], "-t") != 0 || (argc == 4 && strcmp(argv[3], "-m") != 0)) {
        fprintf(stderr, "Wrong arguments!\n");
        return -1;
    }
    srand(time(NULL));
    char *last = NULL;
    long max_threads = strtol(argv[2], &last, 10);
    if (max_threads <= 0 || *last != '\0') {
        fprintf(stderr, "Wrong arguments!\n");
        return -1;
    }
    char *string = NULL, *end;
    size_t len = 0;
    fprintf(stdout, "Enter a size of the first matrix: ");
    ssize_t count = getline(&string, &len, stdin);
    if (count == -1) {
        fprintf(stderr, "An error occurred!\n");
        free(string);
        return -1;
    }
    long rows_1 = strtol(string, &end, 10);
    long cols_1 = strtol(end, NULL, 10);
    if (rows_1 <= 0 || cols_1 <= 0) {
        fprintf(stderr, "Wrong shape of a matrix!\n");
        free(string);
        return -1;
    }
    fprintf(stdout, "Enter a size of the second matrix: ");
    count = getline(&string, &len, stdin);
    if (count == -1) {
        fprintf(stderr, "An error occurred!\n");
        free(string);
        return -1;
    }
    long rows_2 = strtol(string, &end, 10);
    long cols_2 = strtol(end, NULL, 10);
    if (rows_2 <= 0 || cols_2 <= 0) {
        fprintf(stderr, "Wrong shape of a matrix!\n");
        free(string);
        return -1;
    }
    if (cols_1 != rows_2) {
        fprintf(stderr, "Multiplication of the matrices is impossible!\n");
        free(string);
        return -1;
    }
    comp **A = malloc(rows_1 * sizeof(comp *));
    if (A == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        free(string);
        return -1;
    }
    comp **B = malloc(rows_2 * sizeof(comp *));
    if (B == NULL) {
        free(A);
        fprintf(stderr, "Memory allocation failed!\n");
        free(string);
        return -1;
    }
    for (int i = 0; i < rows_1; i++) {
        comp *row = malloc(cols_1 * sizeof(comp));
        if (row == NULL) {
            for (int j = 0; j < i; j++) free(A[j]);
            free(A);
            free(B);
            fprintf(stderr, "Memory allocation failed!\n");
            free(string);
            return -1;
        }
        A[i] = row;
    }

    for (int i = 0; i < rows_2; i++) {
        comp *row = malloc(cols_2 * sizeof(comp));
        if (row == NULL) {
            for (int j = 0; j < i; j++) free(B[j]);
            free(B);
            for (int k = 0; k < rows_1; k++) free(A[k]);
            free(A);
            fprintf(stderr, "Memory allocation failed!\n");
            free(string);
            return -1;
        }
        B[i] = row;
    }

    comp **C = malloc(rows_1 * sizeof(comp *));
    if (C == NULL) {
        for (int j = 0; j < rows_2; j++) free(B[j]);
        free(B);
        for (int k = 0; k < rows_1; k++) free(A[k]);
        free(A);
        fprintf(stderr, "Memory allocation failed!\n");
        free(string);
        return -1;
    }
    for (int k = 0; k < rows_1; k++) {
        comp *row = malloc(sizeof(comp) * cols_2);
        if (row == NULL) {
            for (int i = 0; i < k; i++) free(C[i]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int p = 0; p < rows_1; p++) free(A[p]);
            free(A);
            fprintf(stderr, "Memory allocation failed!\n");
            free(string);
            return -1;
        }
        C[k] = row;
    }
    if (argc == 4) {
        if (matrix_fill(A, rows_1, cols_1) == NULL) {
            for (int i = 0; i < rows_1; i++) free(C[i]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int p = 0; p < rows_1; p++) free(A[p]);
            free(A);
            fprintf(stderr, "Matrix filling failed!\n");
            free(string);
            return -1;
        }

        if (matrix_fill(B, rows_2, cols_2) == NULL) {
            for (int i = 0; i < rows_1; i++) free(C[i]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int p = 0; p < rows_1; p++) free(A[p]);
            fprintf(stderr, "Matrix filling failed!\n");
            free(string);
            free(A);
            return -1;
        }
    } else {
        if (matrix_spawn(A, rows_1, cols_1) == NULL) {
            for (int i = 0; i < rows_1; i++) free(C[i]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int p = 0; p < rows_1; p++) free(A[p]);
            free(A);
            fprintf(stderr, "Matrix generation failed!\n");
            free(string);
            return -1;
        }

        if (matrix_spawn(B, rows_2, cols_2) == NULL) {
            for (int i = 0; i < rows_1; i++) free(C[i]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int p = 0; p < rows_1; p++) free(A[p]);
            fprintf(stderr, "Matrix generation failed!\n");
            free(string);
            free(A);
            return -1;
        }
    }

    long threads_count = max_threads < rows_1 ? max_threads : rows_1;
    pthread_t *threads = malloc(sizeof(pthread_t) * threads_count);
    if (threads == NULL) {
        for (int i = 0; i < rows_1; i++) free(C[i]);
        free(C);
        for (int j = 0; j < rows_2; j++) free(B[j]);
        free(B);
        for (int p = 0; p < rows_1; p++) free(A[p]);
        free(A);
        fprintf(stderr, "Memory allocation failed!\n");
        free(string);
        return -1;
    }
    TD *data = malloc(sizeof(TD) * threads_count);
    if (data == NULL) {
        for (int i = 0; i < rows_1; i++) free(C[i]);
        free(C);
        for (int j = 0; j < rows_2; j++) free(B[j]);
        free(B);
        for (int p = 0; p < rows_1; p++) free(A[p]);
        free(A);
        fprintf(stderr, "Memory allocation failed!\n");
        free(string);
        free(threads);
        return -1;
    }

    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long created = 0;
    for (int i = 0; i < threads_count; i++) {
        data[i].A = A;
        data[i].B = B;
        data[i].C = C;
        data[i].cols_a = cols_1;
        data[i].cols_b = cols_2;
        data[i].start_row = i * rows_1 / threads_count;
        data[i].end_row = (i + 1) * rows_1 / threads_count;
        int p = pthread_create(&threads[i], NULL, worker, &data[i]);
        if (p != 0) {
            for (int j = 0; j < created; j++) pthread_join(threads[j], NULL);
            fprintf(stderr, "A thread wasn't created!\n");
            for (int y = 0; y < rows_1; y++) free(C[y]);
            free(C);
            for (int j = 0; j < rows_2; j++) free(B[j]);
            free(B);
            for (int g = 0; g < rows_1; g++) free(A[g]);
            free(A);
            free(string);
            free(threads);
            free(data);
            return -1;
        }
        created++;
    }

    for (int j = 0; j < threads_count; j++) {
        int p = pthread_join(threads[j], NULL);
        if (p != 0) {
            for (int k = j + 1; k < threads_count; k++) pthread_join(threads[k], NULL);
            fprintf(stderr, "Failed to join a thread!\n");
            return -1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    fprintf(stdout, "Time: %.6f s\n", elapsed);

    if (argc == 4) {
        for (int i = 0; i < rows_1; i++) {
            for (int j = 0; j < cols_2; j++) {
                if (C[i][j].im < 0) fprintf(stdout, "%ld%ldi ", C[i][j].re, C[i][j].im);
                else fprintf(stdout, "%ld+%ldi ", C[i][j].re, C[i][j].im);
            }
            fprintf(stdout, "\n");
        }
    }
    
    for (int y = 0; y < rows_1; y++) free(C[y]);
    free(C);
    for (int j = 0; j < rows_2; j++) free(B[j]);
    free(B);
    for (int g = 0; g < rows_1; g++) free(A[g]);
    free(A);
    free(string);
    free(threads);
    free(data);
    return 0;
}
