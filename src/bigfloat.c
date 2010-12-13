
#include <stdlib.h>
#include <stdio.h>
#include <bigfloat.h>

struct bigfloat_t {
	long double data;
};

bigfloat bigfloat_make(long double d) {
	bigfloat f = (bigfloat) malloc(sizeof(struct bigfloat_t));
	f->data = d;
	return f;
}

void bigfloat_destroy(bigfloat f) {
	free(f);
}

bigfloat bigfloat_add(bigfloat f1, bigfloat f2) {
	return bigfloat_make(f1->data + f2->data);
}

bigfloat bigfloat_sub(bigfloat f1, bigfloat f2) {
	return bigfloat_make(f1->data - f2->data);
}

bigfloat bigfloat_mult(bigfloat f1, bigfloat f2) {
	return bigfloat_make(f1->data * f2->data);
}

bigfloat bigfloat_div(bigfloat f1, bigfloat f2) {
	return bigfloat_make(f1->data / f2->data);
}

bool bigfloat_greater(bigfloat f1, bigfloat f2) {
	return (f1->data > f2->data);
}

bool bigfloat_less(bigfloat f1, bigfloat f2) {
	return (f1->data < f2->data);
}

bigfloat atobf(char * source) {
	bigfloat f = (bigfloat) malloc(sizeof(struct bigfloat_t));
	char * p;
	f->data = strtold(source, &p);
	return f;
}

char * bftoa(bigfloat f) {
	char * r = NULL;
	size_t size = snprintf(r, 0, "%Lg", f->data) + 1;
	r = realloc(r, size * sizeof(char));
	snprintf(r, size, "%Lg", f->data);
	return r;
}

