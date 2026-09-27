#include "../include/child.h"
#include <stdio.h>
#include <stdlib.h>

int child(void) {
    char *string = NULL;
    size_t size = 0;
    while (getline(&string, &size, stdin) != -1) {
        const char *ptr = string;
        char *end;
        long sum = 0;
        while (1) {
            const long number = strtol(ptr, &end, 10);
            if (ptr == end) break;
            sum += number;
            ptr = end;
        }
        fprintf(stdout, "%ld\n", sum);
    }
    free(string);
    return 0;
}

int main(void) {
    return child();
}
