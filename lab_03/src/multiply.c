#include "../include/multiply.h"
#include "../include/structs.h"


comp multiply(comp a, comp b) {
    comp res = {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
    return res;
}
