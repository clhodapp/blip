
#ifndef BUILTINS_H
#define BUILTINS_H

#include <environment.h>

void register_builtins(lexeme env);
lexeme (*builtin_get_action(lexeme b))(lexeme l);
lexeme builtin_get_params(lexeme b);

#endif
