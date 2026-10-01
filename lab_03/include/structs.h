#ifndef OS_LABS_STRUCTS_H
#define OS_LABS_STRUCTS_H

typedef struct Complex {
    long re;
    long im;
} comp;


typedef struct ThreadData {
    comp **A;
    comp **B;
    comp **C;

    long cols_a;
    long cols_b;

    long start_row;
    long end_row;
} TD;

#endif //OS_LABS_STRUCTS_H
