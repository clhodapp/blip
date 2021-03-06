
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <lexeme.h>
#include <stdbool.h>

lexeme env_make();
lexeme env_get_parent(lexeme env); 
lexeme env_extend(lexeme env, lexeme idList, lexeme valueList); // lists use lexeme left pointers and are null terminated
lexeme env_lookup(lexeme env, lexeme id); // value field of lexeme is typename
void env_insert(lexeme env, lexeme id, lexeme value);
lexeme env_alter(lexeme env, lexeme id, lexeme value);
lexeme env_remove(lexeme env, lexeme id);
void env_destroy(lexeme env);
lexeme env_id_list(lexeme env); // get first id for an environment
lexeme env_value_list(lexeme env); // get first value for environment

#endif
