
#ifndef PAIR_H
#define PAIR_H

#include <lexeme.h>

typedef struct pair_t *pair;
pair pair_make(lexeme left, lexeme right);
void pair_destroy(pair p);
lexeme pair_get_left(pair p);
lexeme pair_get_right(pair p);

#endif
