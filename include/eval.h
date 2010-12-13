
#ifndef EVAL_H
#define EVAL_H

#include <lexeme.h>

void eval_init(); // must be run before using eval
lexeme eval(lexeme env, lexeme l);

#endif
