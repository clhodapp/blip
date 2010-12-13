#ifndef LEX_H
#define LEX_H

#include <lexeme.h>
#include <bigint.h>
#include <stdio.h>

typedef struct lex_stream_t *lex_stream;

lex_stream lex_stream_open(char *path);
lex_stream lex_stream_open_file(FILE * f);
void lex_stream_close(lex_stream);
bigint lex_stream_get_linenum(lex_stream l);
char lex_stream_getc(lex_stream l);
char lex_stream_ungetc(char ungotten, lex_stream l);
lexeme lex(lex_stream l);
void unlex(lex_stream target, lexeme lm);

#endif
