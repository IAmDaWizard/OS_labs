#include <stdio.h>
#include <pthread.h>
#include "../include/structs.h"
#include "../include/multiply.h"
#include <stdlib.h>

// void *worker(void *arg) {
//     fprintf(stdout, "Hello from worker %d!\n", *(int *) arg);
//     return NULL;
// }
//
// int main(void) {
//     fprintf(stdout, "Hello from main thread!\n");
//     pthread_t thread[5];
//     int num[5];
//     for (int x = 0; x < 5; x++) {
//         num[x] = x;
//         pthread_create(&thread[x], NULL, worker, &num[x]);
//     }
//     for (int x = 0; x < 5; x++) {
//         pthread_join(thread[x], NULL);
//     }
//     fprintf(stdout, "Worker finished!\n");
//     return 0;
// }

int main(void) {
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






    // for (int i = 0; i < 2; i++) {
    //     for (int j = 0; j < 2; j++) {
    //         comp num = {5, 5};
    //         A[i][j] = num;
    //         B[i][j] = multiply(num, num);
    //     }
    // }
    // comp C[2][2];
    // for (int i = 0; i < 2; i++) {
    //     for (int j = 0; j < 2; j++) {
    //         comp res = {0, 0};
    //         for (int k = 0; k < 2; k++) {
    //             comp mlt = multiply(A[i][k], B[k][j]);
    //             res.re += mlt.re;
    //             res.im += mlt.im;
    //         }
    //         C[i][j] = res;
    //     }
    // }
    return 0;
}
