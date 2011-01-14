
#ifndef BIGINT_H
#define BIGINT_H

#include <stdbool.h>

typedef enum {BIGINT_POS, BIGINT_NEG} bigint_sign ;

typedef struct bigint_t {
	bigint_sign sign;
	long long unsigned int data;
} *bigint;

void bigint_init(); // must be called before using bigints
bigint bigint_make(long long int i);
bigint bigint_duplicate(bigint i);
bigint bigint_add(bigint i1, bigint i2);
bigint bigint_sub(bigint i1, bigint i2);
bigint bigint_mult(bigint i1, bigint i2);
bigint bigint_div(bigint i1, bigint i2);
bigint bigint_mod(bigint i1, bigint i2);
bigint bigint_inc(bigint i);
bigint bigint_dec(bigint i);
bool bigint_greater(bigint i1, bigint i2);
bool bigint_less(bigint i1, bigint i2);
bool bigint_equal(bigint i1, bigint i2);
char * bitoa(bigint i);
bigint atobi(char * source);
void bigint_destroy(bigint i);

#endif
