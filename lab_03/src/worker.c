#include "../include/worker.h"
#include "../include/structs.h"
#include "../include/multiply.h"
#include <stdio.h>

void *worker(void *arg) {
    TD *data = (TD *) arg;

    for (long i = data->start_row; i < data->end_row; i++) {
        for (long j = 0; j < data->cols_b; j++) {
            comp res = {0, 0};
            for (long k = 0; k < data->cols_a; k++) {
                comp mlt = multiply(data->A[i][k], data->B[k][j]);
                res.re += mlt.re;
                res.im += mlt.im;
            }
            data->C[i][j] = res;
        }
    }
    return NULL;
}
