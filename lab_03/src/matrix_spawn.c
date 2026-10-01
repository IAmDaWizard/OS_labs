#include "../include/matrix_spawn.h"
#include "../include/structs.h"
#include <stdlib.h>


comp **matrix_spawn(comp **matrix, long rows, long cols) {
    if (rows <= 0 || cols <= 0) return NULL;
    if (matrix == NULL) return NULL;
    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            int re = rand() % 444;
            int im = rand() % 44;
            comp num = {re, im};
            matrix[i][j] = num;
        }
    }
    return matrix;
}
