#include <stdlib.h>
#include <darray.h>

struct darray_t {
	int size;
	int capacity;
	void **data;
};

static void ** darray_resize_data(darray const d, const int new_capacity);

darray darray_make() {
	darray d = (darray) malloc(sizeof(struct darray_t));
	d->size = 0;
	d->capacity = 5;
	d->data = (void **) malloc(5 * sizeof(void *));
	return d;
}

void * darray_add(darray const d, void * const inserted) {
	if (d->size >= d->capacity) {
		d->capacity = d->capacity * 2;
		d->data = (void **) darray_resize_data(d, d->capacity);
	}
	d->data[d->size] = inserted;
	d->size = d->size + 1;
	return inserted;
}

void * darray_get(darray const d, int const index) {
	return d->data[index];
}

void * darray_set(darray const d, int const index, void * const value) {
	d->data[index] = value;
	return value;
}

int darray_grow(darray const d, int const grow_by) {
	d->size = d->size + grow_by;
	if (d->capacity < d->size) {
		do {
			d->capacity = d->capacity * 2;
		} while (d->capacity < d->size);
		darray_resize_data(d, d->capacity);
	}

	return grow_by;
}
int darray_shrink(darray const d, int const reduce_by) {
	d->size = d->size - reduce_by;
	if ((d->capacity >= 20) && (d->size < (d->capacity / 4))) {
		do {
			d->capacity = d->capacity / 4;
		} while ((d->capacity >= 20) && (d->size < (d->capacity / 4)));
		darray_resize_data(d, d->capacity);
	}

	return reduce_by;
}

int darray_capacity(darray const d) {
	return d->capacity;
}

int darray_size(darray const d) {
	return d->size;
}

void darray_destroy(darray const d) {
	free(d->data);
	free(d);
}

static void ** darray_resize_data(darray const d, int const new_capacity) {
	return (void **) realloc(d->data, new_capacity * sizeof(void *));
}

