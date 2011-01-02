#ifndef LEXEME_H
#define LEXEME_H

#include <stdbool.h>
#include <bigint.h>

typedef enum {
	LEXEME_TYPE_MIN, // used as sentinal; Don't put anything before this
	
	COMMA,
	END_OF_FILE,
	OBRACE,
	CBRACE,
	OPAREN,
	CPAREN,
	AMP,
	DOT,
	BIND,
	LAMBDA,
	TRUE,
	FALSE,
	NIL,
	RETURN,
	SEMI,
	INVALID_CHAR,
	INT,
	STRING,
	DEC,
	ID,
	IMPORT,

	//start artificial lexemes:
	ARGLIST,
	BLOCK,
	CALL,
	PARAMLIST,
	RETURNFORM,
	UNITLIST,
	ENV,
	FRAME,
	FUNC_OBJ,
	FUNC_OBJ_SPACER,
	BUILTIN,
	BUILTIN_SPACER,
	LIST_SPACER,
	VAL_LIST,
	ID_LIST,
	TYPE,
	PAIR,
	ACTION,

	LEXEME_TYPE_MAX // used as sentinal; Don't put anything after this
} lexeme_type;

typedef struct lexeme_t *lexeme;

lexeme lexeme_make(lexeme_type type);
lexeme_type lexeme_get_type(lexeme l);
char * lexeme_get_typename(lexeme l);
//lexeme_type lexeme_type_from_string(char *value);
char * lexeme_type_to_string(lexeme_type t);
char * lexeme_type_natural_name(lexeme_type t);
void * lexeme_get_data(lexeme l);
lexeme_type lexeme_set_type(lexeme l, lexeme_type t);
void * lexeme_set_data(lexeme l, void *d);
void lexeme_destroy(lexeme l);
lexeme_type lexeme_recognize_special(char *c);
void lexeme_overwrite(lexeme overwritten, lexeme overwriter);
bigint lexeme_get_linenum(lexeme l);
bigint lexeme_set_linenum(lexeme l, bigint linenum);
lexeme lexeme_copy(lexeme l);
char * lexeme_to_string(lexeme stringified);
#endif

