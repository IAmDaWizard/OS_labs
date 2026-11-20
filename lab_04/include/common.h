#ifndef LAB_04_COMMON_H
#define LAB_04_COMMON_H
#include <stddef.h>

typedef enum {
    MESSAGE_NONE = 0,
    MESSAGE_RESULT = 1,
    MESSAGE_DONE = 2,
    MESSAGE_ERROR = 3
} MessageType;


typedef struct {
    MessageType type;
    size_t line_number;
    long long sum;
    char error_text[256];
} SharedData;

#endif
