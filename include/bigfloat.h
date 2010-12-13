
#ifndef BIGFLOAT_H
#define BIGFLOAT_H

#include <stdbool.h>

typedef struct bigfloat_t *bigfloat;

bigfloat bigfloat_make(long double d);
void bigfloat_destroy(bigfloat f);
bigfloat bigfloat_add(bigfloat f1, bigfloat f2);
bigfloat bigfloat_sub(bigfloat f1, bigfloat f2);
bigfloat bigfloat_mult(bigfloat f1, bigfloat f2);
bigfloat bigfloat_div(bigfloat f1, bigfloat f2);
bool bigfloat_greater(bigfloat f1, bigfloat f2);
bool bigfloat_less(bigfloat f1, bigfloat f2);
bigfloat atobf(char * source);
char * bftoa(bigfloat f);

#endif
