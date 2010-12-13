
#include <inttypes.h>
#include <bigint.h>
#include <stdlib.h>
#include <stdio.h>

void bigint_init() {
	return;
}

bigint bigint_make(long long int i) {
	bigint r = malloc(sizeof(struct bigint_t));
	if (i >= 0) {
		r->sign = BIGINT_POS;
		r->data = (long long unsigned int) i;
	}
	else {
		r->sign = BIGINT_NEG;
		r->data = (long long unsigned int) -i;
	}
	return r;
}

bigint bigint_duplicate(bigint i) {
	bigint r = malloc(sizeof(struct bigint_t));
	r->sign = i->sign;
	r->data = i->data;
	return r;
}

bigint bigint_add(bigint i1, bigint i2) {
	bigint r = malloc(sizeof(struct bigint_t));
	if (i1->sign == i2->sign) {
		r->sign = i1->sign;
		r->data = i1->data + i2->data;
	}
	else if (i1->data > i2->data) {
		r->sign = i1->sign;
		r->data = i1->data - i2->data;
	}
	else {
		r->sign = i2->sign;
		r->data = i2->data - i1->data;
	}
	if (r->data == 0) r->sign = BIGINT_POS;
	return r;
}

bigint bigint_sub(bigint i1, bigint i2) {
	bigint addable = bigint_duplicate(i2);
	if (addable->sign == BIGINT_NEG) {
		addable->sign = BIGINT_POS;
	}
	else {
		addable->sign = BIGINT_NEG;
	}
	bigint returnable = bigint_add(i1, addable);
	bigint_destroy(addable);
	return returnable;
}

bigint bigint_mult(bigint i1, bigint i2) {
	bigint r = malloc(sizeof(struct bigint_t));
	r->data = i1->data * i2->data;
	if (i1->sign == i2->sign) {
		r->sign = BIGINT_POS;
	}
	else {
		r->sign = BIGINT_NEG;
	}
	return r;
}

bigint bigint_div(bigint i1, bigint i2) {
	bigint r = malloc(sizeof(struct bigint_t));
	r->data = i1->data / i2->data;
	if (i1->sign == i2->sign) {
		r->sign = BIGINT_POS;
	}
	else {
		r->sign = BIGINT_NEG;
	}
	return r;
}

bigint bigint_mod(bigint i1, bigint i2) {
	bigint r = malloc(sizeof(struct bigint_t));
	r->data = i1->data % i2->data;
	r->sign = i1->sign;
	return r;
}

bool bigint_greater(bigint i1, bigint i2) {
	if (i1->sign == BIGINT_POS && i2->sign == BIGINT_NEG) {
		return true;
	}
	else if (i2->sign == BIGINT_POS && i1->sign == BIGINT_NEG) {
		return false;
	} // i1, i2 have same signs after here
	else if (i1->sign == BIGINT_POS) {
		return (i1->data > i2->data);
	}
	else
		return (i1->data < i2->data);
}

bool bigint_less(bigint i1, bigint i2) {
	if (i1->sign == BIGINT_POS && i2->sign == BIGINT_NEG) {
		return false;
	}
	else if (i2->sign == BIGINT_POS && i1->sign == BIGINT_NEG) {
		return true;
	} // i1, i2 have same signs after here
	else if (i1->sign == BIGINT_POS) {
		return (i1->data < i2->data);
	}
	else
		return (i1->data > i2->data);
}

bool bigint_equal(bigint i1, bigint i2) {
	return (i1->sign == i2->sign && i1->data == i2->data);
}

char * bitoa(bigint i) {
	char * returned = NULL;
	char * dest;
	size_t size = snprintf(returned, 0, "%llu", i->data) + 1;
	if (i->sign == BIGINT_NEG) {
		returned = malloc((size+1)*sizeof(char));
		returned[0] = '-';
		dest = returned + 1;
	}
	else {
		returned = malloc(size*sizeof(char));
		dest = returned;
	}
	sprintf(dest, "%llu", i->data);
	return returned;
}

bigint atobi(char * source) {
	bigint r = malloc(sizeof(struct bigint_t));
	char ch = *source;
	if (ch == '-') {
		r->sign = BIGINT_NEG;
		++source;
	}
	else {
		r->sign = BIGINT_POS;
	}
	char * p = 0;
	r->data = strtoull(source, &p, 10);
	return r;
}

void bigint_destroy(bigint i) {
	free(i);
}
