#include "../include/matrix_fill.h"
#include "../include/structs.h"
#include <stdio.h>
#include <stdlib.h>

comp **matrix_fill(comp **matrix, long rows, long cols) {
    if (matrix == NULL || rows <= 0 || cols <= 0) return NULL;
    char *string = NULL;
    size_t len = 0;
    fprintf(stdout, "\n");
    fprintf(stdout, "Fill your matrix below:\n");
    for (long i = 0; i < rows; i++) {
        ssize_t count = getline(&string, &len, stdin);
        if (count == -1) {
            fprintf(stderr, "An error occurred :(\n");
            free(string);
            return NULL;
        }
        char *ptr = string;
        for (long j = 0; j < cols; j++) {
            char *end;
            long re = strtol(ptr, &end, 10);
            long im = strtol(end, &end, 10);
            if (*end != 'i') {
                fprintf(stderr, "You entered a number of a wrong format.\n");
                free(string);
                return NULL;
            }
            matrix[i][j].re = re;
            matrix[i][j].im = im;
            ptr = end + 1;
        }
    }
    fprintf(stdout, "\n");
    free(string);
    return matrix;
}
