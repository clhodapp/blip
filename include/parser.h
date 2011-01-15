
#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <lex.h>

lexeme parse(lex_stream l);
lexeme make_paramlist(char * listString);

#endif
