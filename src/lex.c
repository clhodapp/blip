#include <stdlib.h>
#include <stdio.h>
#include <lex.h>
#include <ctype.h>
#include <darray.h>
#include <lexeme.h>
#include <bigint.h>
#include <bigfloat.h>
#include <lexemetypes.h>
#include <stdbool.h>
#include <stddef.h>

struct lex_stream_t{
	FILE *sourceFile;
	bigint linenum;
	lexeme unlexed;
};

static bigint one;
static int streamcount = 0;
static void lex_stream_advance_linenum(lex_stream l);

lex_stream lex_stream_open(char *path) {
	FILE * source = fopen(path, "r");

	if (source == NULL) {
		return NULL;
	}
	return lex_stream_open_file(source);

}

lex_stream lex_stream_open_file(FILE * f) {

	if (streamcount == 0) {
		one = bigint_make(1);
	}
	streamcount++;
	lex_stream r = (lex_stream) malloc(sizeof(struct lex_stream_t));
	r->sourceFile = f;
	r->linenum = bigint_make(1);
	r->unlexed = NULL;
	return r;
}

void lex_stream_close(lex_stream l) {
	lexeme lm = l->unlexed;
	while (lm != NULL) {
		lexeme tmp = lm;
		lm = lexeme_get_right(lm);
		lexeme_destroy(tmp);
	}
	streamcount--;
	fclose(l->sourceFile);
	bigint_destroy(l->linenum);
	free(l);
	if (streamcount==0) {
		bigint_destroy(one);
	}
}

static void remove_whitespace(lex_stream source);
static bool strip_comment(lex_stream source);
static lexeme get_numeric(lex_stream source);
static lexeme get_string(lex_stream source);
static lexeme get_alpha(lex_stream source);
static lexeme get_logical(lex_stream source);
static bool is_id_char(char ch);

lexeme lex(lex_stream source) {
	lexeme unlexed = source->unlexed;
	if (unlexed != NULL) {
		lexeme returned = unlexed;
		source->unlexed = lexeme_get_right(unlexed);
		return returned;
	}

	remove_whitespace(source);

	char current = lex_stream_getc(source);

	lexeme l;

	switch(current) {
	case ',':
		l = lexeme_make(COMMA);
		break;
	case '{':
		l =  lexeme_make(OBRACE);
		break;
	case '}':
		l = lexeme_make(CBRACE);
		break;
	case '(':
		l = lexeme_make(OPAREN);
		break;
	case ')':
		l = lexeme_make(CPAREN);
		break;
	case '&':
		l = lexeme_make(AMP);
		break;
	case '.':
		l = lexeme_make(DOT);
		break;
	case ';':
		l = lexeme_make(SEMI);
		break;
	case EOF:
		l = lexeme_make(END_OF_FILE);
		break;
	default:
		if (current == '-')
		{
			char tmp = lex_stream_getc(source);
			lex_stream_ungetc(tmp, source);
			if (isdigit(tmp)) {
				lex_stream_ungetc(current, source);
				l = get_numeric(source);
				break;
			}
		}
		if (isdigit(current)) {
			lex_stream_ungetc(current, source);
			l = get_numeric(source);
			break;
		}
		else if (is_id_char(current))
		{
			lex_stream_ungetc(current, source);
			l = get_alpha(source);
		}
		else if (current == '"')
		{
			lex_stream_ungetc(current, source);
			l = get_string(source);
		}
		else if (current == '#')
		{
			lex_stream_ungetc(current, source);
			l = get_logical(source);
		}
		else {
			l = lexeme_make(INVALID_CHAR);
		}
	}
	lexeme_set_linenum(l, lex_stream_get_linenum(source));
	return l;
}

void unlex(lex_stream target, lexeme lm) {
	lexeme_set_right(lm, target->unlexed);
	target->unlexed = lm;
}

bigint lex_stream_get_linenum(lex_stream l) {
	return l->linenum;
}

void lex_stream_advance_linenum(lex_stream l) {
	bigint newLineNum = bigint_add(l->linenum, one);
	bigint_destroy(l->linenum);
	l->linenum = newLineNum;
}

char lex_stream_getc(lex_stream l) {
	return getc(l->sourceFile);
}

char lex_stream_ungetc(char ungotten, lex_stream l) {
	return ungetc(ungotten, l->sourceFile);
}

static void remove_whitespace(lex_stream source) {
	char current;
	bool done = false;

	while (!done)
	{
		current = lex_stream_getc(source);
		if (current == '\n') {
			lex_stream_advance_linenum(source);
		}
		if (!isspace(current))
		{
			lex_stream_ungetc(current, source);
			done = !strip_comment(source); //done if not a comment
		}
	}
}

static bool strip_comment(lex_stream source) {
	char c1;
	char c2;
	
	c1 = lex_stream_getc(source);
	if (c1 == '/')
	{
		c2 = lex_stream_getc(source);
		if (c2 == '/')
		{
			while (lex_stream_getc(source) != '\n');
			lex_stream_advance_linenum(source);
			return true;
		}
		lex_stream_ungetc(c2, source);
	}
	lex_stream_ungetc(c1, source);
	return false;
}

static lexeme get_numeric(lex_stream source) {
	char * numstring = (char *) malloc(sizeof(char));
	numstring[0] = lex_stream_getc(source);
	size_t size = (size_t) 1;
	bool decimal = false;
	char ch;
	while (isdigit(ch = lex_stream_getc(source)) || ch == '.') {
		if (ch == '.') {
			if (decimal) {
				lex_stream_ungetc('.', source);
				break;
			}
			decimal = true;
		}
		numstring = (char *) realloc(numstring, (++size) * sizeof(char));
		numstring[size - (size_t) 1] = ch;
	}
	lex_stream_ungetc(ch, source);
	numstring = realloc(numstring, (size + 1) * sizeof(char));
	numstring[size] = 0;
	lexeme r;
	if (!decimal) {
		r = lexeme_make(INT);
		lexeme_set_data(r, atobi(numstring));
	}
	else {
		r = lexeme_make(DEC);
		lexeme_set_data(r, atobf(numstring));
	}
	free(numstring);
	return r;
}

static lexeme get_alpha(lex_stream source) {
	char ch;
	darray d = darray_make();
	while (is_id_char(ch = lex_stream_getc(source)) || isdigit(ch)) {
		char *c = malloc(sizeof(char));
		*c = ch;
		darray_add(d, c);
	}
	lex_stream_ungetc(ch, source);
	char *value = malloc((darray_size(d) + 1) * sizeof(char));
	for (int i = 0; i < darray_size(d); i++) {
		value[i] = * ((char *) darray_get(d, i));
	}
	value[darray_size(d)] = 0;
	for (int i=0; i<darray_size(d); i++) {
		free(darray_get(d, i));
	}
	darray_destroy(d);
	lexeme_type type = lexeme_recognize_special(value);
	lexeme l = lexeme_make(type);
	if (type == ID) {
		lexeme_set_data(l, value);
	}
	else {
		free(value);
	}
	return l;
}

static lexeme get_string(lex_stream source) {
	char ch = lex_stream_getc(source); // gets double quote;
	darray d = darray_make();
	while ((ch = lex_stream_getc(source)) != '"' && ch != EOF) {
		if (ch == '\n') lex_stream_advance_linenum(source);
		if (ch == '\\') {
			ch = lex_stream_getc(source);
			switch (ch)
			{
				case 'n':
					ch = '\n';
					break;
				case 'r':
					ch = '\r';
					break;
				case 't':
					ch = '\t';
					break;
				case 'v':
					ch = '\v';
					break;
				case 'b':
					ch = '\b';
					break;
				case 'a':
					ch = '\a';
					break;
				case 'f':
					ch = '\f';
					break;
				case '0':
					ch = '\0';
					break;
			}
			if (ch == '\n') lex_stream_advance_linenum(source);
		}
		char *c = malloc(sizeof(char));
		(*c) = ch;
		darray_add(d, c);
	}
	char *value = malloc((darray_size(d) + 1) * sizeof(char));
	for (int i=0; i<darray_size(d); i++) {
		value[i] = * ((char *) darray_get(d, i));
	}
	value[darray_size(d)] = 0;
	for (int i=0; i<darray_size(d); i++) {
		free(darray_get(d, i));
	}
	darray_destroy(d);
	lexeme l = lexeme_make(STRING);
	lexeme_set_data(l, value);
	return l;
}

static lexeme get_logical(lex_stream source) {
	char ch = lex_stream_getc(source); // gets #
	ch = lex_stream_getc(source);
	switch (ch)
	{
		case 't':
			return lexeme_make(TRUE);
		case 'f':
			return lexeme_make(FALSE);
		case 'n':
			return lexeme_make(NIL);
		default:
			lex_stream_ungetc(ch, source);
			return lexeme_make(INVALID_CHAR);
	}
}

static bool is_id_char(char ch) {
	return (isalpha(ch) || (ch == '+') || (ch == '-') || (ch == '/') ||
			(ch == '|') || (ch == '*') || (ch == '%') || (ch == '?') || (ch == '!'));
}
