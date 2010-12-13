
#ifndef DARRAY_H
#define DARRAY_H


struct darray_t;
typedef struct darray_t *darray;

darray darray_make();
void * darray_add(darray d, void *inserted);
void * darray_get(darray d, int index);
void * darray_set(darray d, int index, void *value);
int darray_grow(darray d, int grow_by);
int darray_shrink(darray d, int reduce_by);
int darray_capacity(darray d);
int darray_size(darray d);
void darray_destroy(darray d);

#endif

