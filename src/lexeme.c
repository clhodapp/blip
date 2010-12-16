
#include <stdlib.h>

#include <lexeme.h>
#include <bigint.h>
#include <bigfloat.h>
#include <lexemetypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pair.h>

struct lexeme_t;
static char * naturalNames[];
static lexeme unique_lexemes[];
static char * (*stringifiers[])(lexeme);
static void (*data_destroyers[])(void *);

static struct lexeme_t s_nil;
static struct lexeme_t s_true;
static struct lexeme_t s_false;
static struct lexeme_t s_semi;
static struct lexeme_t s_oparen;
static struct lexeme_t s_cparen;
static struct lexeme_t s_obrace;
static struct lexeme_t s_cbrace;
static struct lexeme_t s_invalid_char;

static char * to_string_default(lexeme l);
static char * to_string_int(lexeme l);
static char * to_string_dec(lexeme l);
static char * to_string_string(lexeme l);
static char * to_string_id(lexeme l);
static char * to_string_true(lexeme l);
static char * to_string_false(lexeme l);
static char * to_string_nil(lexeme l);
static char * to_string_list(lexeme l);
static char * to_string_type(lexeme l);
static char * to_string_invalid_char(lexeme l);
static char * to_string_invalid_char(lexeme l);
static char * to_string_import(lexeme l);
static char * to_string_amp(lexeme l);
static char * to_string_dot(lexeme l);
static char * to_string_return(lexeme l);
static char * to_string_bind(lexeme l);
static char * to_string_oparen(lexeme l);
static char * to_string_cparen(lexeme l);
static char * to_string_obrace(lexeme l);
static char * to_string_cbrace(lexeme l);
static char * to_string_lambda(lexeme l);
static char * to_string_semi(lexeme l);
static char * to_string_pair(lexeme l);

static void i_bigint_destroy(void *);
static void i_bigfloat_destroy(void *);
static void i_pair_destroy(void *);

struct lexeme_t {
	lexeme_type type;
	bool data_initialized;
	bool unique;
	void *data;
	bigint linenum;
	lexeme left;
	lexeme right;
};

static char * naturalNames[LEXEME_TYPE_MAX + 1] = {
	[LEXEME_TYPE_MIN] = "lexeme minimum sentinal",
	[LEXEME_TYPE_MAX] = "lexeme maximim sentinal",
	[LIST] = "list",
	[COMMA] = "comma",
	[END_OF_FILE] = "end of file",
	[OBRACE] = "opening brace",
	[CBRACE] = "closing brace",
	[OPAREN] = "opening parenthesis",
	[CPAREN] = "closing parenthesis",
	[IMPORT] = "import",
	[AMP] = "ampersand",
	[DOT] = "dot",
	[BIND] = "bind",
	[LAMBDA] = "lambda",
	[TRUE] = "true",
	[FALSE] = "false",
	[NIL] = "nil",
	[RETURN] = "return",
	[SEMI] = "semicolon",
	[INVALID_CHAR] = "invalid character",
	[INT] = "integer",
	[STRING] = "string",
	[DEC] = "decimal",
	[ID] = "id",
	[ARGLIST] = "argument list",
	[BLOCK] = "statement block",
	[CALL] = "function call",
	[PARAMLIST] = "parameter list",
	[RETURNFORM] = "return form",
	[UNITLIST] = "unit list",
	[FUNC_OBJ] = "function object",
	[FUNC_OBJ_SPACER] = "function object spacer",
	[ENV] = "environment",
	[FRAME] = "frame",
	[LIST_SPACER] = "list spacer",
	[BUILTIN_SPACER] = "builtin spacer",
	[BUILTIN] = "builtin",
	[TYPE] = "type",
	[PAIR] = "pair"
};

static void (*data_destroyers[LEXEME_TYPE_MAX + 1])(void * destroyed) = {
	[INT] = &i_bigint_destroy,
	[DEC] = &i_bigfloat_destroy,
	[PAIR] = &i_pair_destroy
};

void i_bigint_destroy(void * destroyed) {
	bigint_destroy((bigint) destroyed);
}

void i_bigfloat_destroy(void * destroyed) {
	bigfloat_destroy((bigfloat) destroyed);
}

void i_pair_destroy(void * destroyed) {
	pair_destroy((pair) destroyed);
}

static struct lexeme_t s_nil = {
	.type = NIL
};

static struct lexeme_t s_true = {
	.type = TRUE
};

static struct lexeme_t s_false = {
	.type = FALSE
};

static struct lexeme_t s_semi = {
	.type = SEMI
};

static struct lexeme_t s_oparen = {
	.type = OPAREN
};

static struct lexeme_t s_cparen = {
	.type = CPAREN
};

static struct lexeme_t s_obrace = {
	.type = OBRACE
};

static struct lexeme_t s_cbrace = {
	.type = CBRACE
};

static struct lexeme_t s_eof = {
	.type = END_OF_FILE
};

static lexeme unique_lexemes[LEXEME_TYPE_MAX + 1] = {
	[INVALID_CHAR] = &s_invalid_char,
	[CBRACE] = &s_cbrace,
	[OBRACE] = &s_obrace,
	[CPAREN] = &s_cparen,
	[OPAREN] = &s_oparen,
	[SEMI] = &s_semi,
	[FALSE] = &s_false,
	[TRUE] = &s_true,
	[NIL] = &s_nil,
	[END_OF_FILE] = &s_eof
};

static char * (*stringifiers[LEXEME_TYPE_MAX + 1])(lexeme l) = {
	[PAIR] = &to_string_pair,
	[INT] = &to_string_int,
	[STRING] = &to_string_string,
	[DEC] = &to_string_dec,
	[ID] = &to_string_id,
	[TRUE] = &to_string_true,
	[FALSE] = &to_string_false,
	[NIL] = &to_string_nil,
	[LIST] = &to_string_list,
	[TYPE] = &to_string_type,
	[LAMBDA] = &to_string_lambda,
	[OPAREN] = &to_string_oparen,
	[CPAREN] = &to_string_cparen,
	[OBRACE] = &to_string_obrace,
	[CBRACE] = &to_string_cbrace,
	[IMPORT] = &to_string_import,
	[DOT] = &to_string_dot,
	[AMP] = &to_string_amp,
	[INVALID_CHAR] = &to_string_invalid_char,
	[RETURN] = &to_string_return,
	[BIND] = &to_string_bind,
	[SEMI] = &to_string_semi,
	[UNITLIST] = &to_string_pair

};

lexeme lexeme_make(lexeme_type type) {
	lexeme unique = unique_lexemes[type];
	if (unique != NULL) {
		return unique;
	}
	else {
		lexeme l = (lexeme) malloc(sizeof(struct lexeme_t));
		l->type = type;
		l->data_initialized = false;
		l->left = NULL;
		l->right = NULL;
		l->unique = false;
		return l;
	}
}

lexeme lexeme_copy(lexeme l) {
	lexeme r = (lexeme) malloc(sizeof(struct lexeme_t));
	*r = *l;
	return r;
}

lexeme_type lexeme_get_type(lexeme l) {
	return l->type;
}

char * lexeme_get_typename(lexeme l) {
	char * name =  naturalNames[l->type];
	if (name != NULL) {
		return name;
	}
	else {
		return "unnamed";
	}
}

//lexeme_type lexeme_type_from_string(char *c) {
//	lexeme_type i;
//	char **  types = lexemetypes();
//	for (i = 0; strcmp(c, types[i]) && i < ID; i++);
//	return i;
//}

char * lexeme_type_to_string(lexeme_type t) {
	char * name =  naturalNames[t];
	if (name != NULL) {
		return name;
	}
	else {
		return "unnamed";
	}
}

lexeme_type lexeme_recognize_special(char *c) {
	if (!strcmp(c, "bind"))
		return BIND;
	else if (!strcmp(c, "lambda"))
		return LAMBDA;
	else if (!strcmp(c, "return"))
		return RETURN;
	else if (!strcmp(c, "import"))
		return IMPORT;
	else
		return ID;
}

void * lexeme_get_data(lexeme l) {
	return l->data;
}

lexeme_type lexeme_set_type(lexeme l, lexeme_type t) {
	l->type = t;
	return t;
}

void * lexeme_set_data(lexeme l, void *d) {
	l->data = d;
	l->data_initialized = true;
	return d;
}

void lexeme_destroy(lexeme l) {
	void (*data_destroyer) (void * destroyed);
	if (unique_lexemes[l->type] == NULL) {
		data_destroyer = data_destroyers[l->type];
		if (data_destroyer != NULL) {
			data_destroyer(l->data);
		}
		free(l);
	}
}

void lexeme_recursive_destroy(lexeme l) {
	lexeme left = lexeme_get_left(l);
	lexeme right = lexeme_get_right(l);
	lexeme_destroy(l);
	if (left != NULL) {
		lexeme_recursive_destroy(left);
	}
	if (right != NULL) {
		lexeme_recursive_destroy(right);
	}
}

bigint lexeme_get_linenum(lexeme l) {
	return l->linenum;
}

bigint lexeme_set_linenum(lexeme l, bigint linenum) {
	l->linenum = linenum;
	return linenum;
}


lexeme lexeme_get_left(lexeme l) {
	return l->left;
}

lexeme lexeme_set_left(lexeme modified, lexeme newLeft) {
	modified->left = newLeft;
	return newLeft;
}

lexeme lexeme_get_right(lexeme l) {
	return l->right;
}

lexeme lexeme_set_right(lexeme modified, lexeme newRight) {
	modified->right = newRight;
	return newRight;
}

void lexeme_overwrite(lexeme overwritten, lexeme overwriter) {
	*overwritten = *overwriter;
}

char * lexeme_to_string(lexeme l) {
	char * (* stringifier)(lexeme) = stringifiers[l->type];
	if (stringifier == NULL) {
		return to_string_default(l);
	}
	else {
		return stringifier(l);
	}
}

lexeme lexeme_deep_copy(lexeme l) {
	if (l == NULL) return NULL;
	lexeme r = malloc(sizeof(struct lexeme_t));
	*r = *l;
	r->left = lexeme_deep_copy(l->left);
	r->right = lexeme_deep_copy(l->right);
	return r;
}

static char * to_string_default(lexeme l) {
	int length = 0;
	char * name = naturalNames[l->type];
	char * returned = 0;
	char * format = "<- %s at %p ->";
	length = snprintf(returned, 0, format , name, l);
	++length;
	returned = malloc(length * sizeof(char));
	snprintf(returned, length, format, name, l);
	return returned;
}

static char * to_string_int(lexeme l) {
	return bitoa(l->data);
}

static char * to_string_dec(lexeme l) {
	return bftoa(l->data);
}

static char * to_string_string(lexeme l) {
	return (char *) l->data;
}

static char * to_string_id(lexeme l) {
	return (char *) l->data;
}

static char * to_string_true(lexeme l) {
	return "#t";
}


static char * to_string_false(lexeme l) {
	return "#f";
}

static char * to_string_nil(lexeme l) {
	return "#n";
}

static char * to_string_invalid_char(lexeme l) {
	char *c = malloc(2 * sizeof(char));
	c[0] = *((char *) l->data);
	c[1] = 0;
	return c;
}

static char * to_string_import(lexeme l) {
	return "import";
}

static char * to_string_amp(lexeme l) {
	return "&";
}

static char * to_string_dot(lexeme l) {
	return ".";
}

static char * to_string_return(lexeme l) {
	return "return";
}

static char * to_string_bind(lexeme l) {
	return "bind";
}

static char * to_string_oparen(lexeme l) {
	return "(";
}

static char * to_string_cparen(lexeme l) {
	return ")";
}

static char * to_string_obrace(lexeme l) {
	return "{";
}

static char * to_string_cbrace(lexeme l) {
	return "}";
}

static char * to_string_lambda(lexeme l) {
	return "lambda";
}

static char * to_string_semi(lexeme l) {
	return ";";
}

static char * to_string_pair(lexeme l) {
	char * returned = malloc(2 * sizeof(char)); // null-terminated string "("
	strncpy(returned, "(", 2);
	int size = 2;
	int extension;
	const int COMMA_SPACE = 2; // space for a ", "
	const int SPACE_DOT_SPACE = 3; // space for a " . "
	pair currentPair = (pair) lexeme_get_data(l);
	lexeme left;
	lexeme right;
	do {
		left = pair_get_left(currentPair);
		right = pair_get_right(currentPair);
		extension = strlen(lexeme_to_string(left));
		returned = realloc(returned, (size + extension + 1) * sizeof(char));
		strncpy((returned + size - 1), lexeme_to_string(left), (extension + 1));
		size = size + extension;

		if (lexeme_get_type(right) == PAIR) {
			currentPair = (pair) lexeme_get_data(right);
			returned = realloc(returned, (size + COMMA_SPACE + 1) * sizeof(char));
			strncpy((returned + size - 1), ", ", 2);
			size = size + COMMA_SPACE;
		}
		else {
			currentPair = NULL;
		}

	} while (currentPair != NULL);
	if (lexeme_get_type(right) != NIL) {
		extension = strlen(lexeme_to_string(right));
		returned = realloc(returned, (size + extension + SPACE_DOT_SPACE) * sizeof(char));
		strncpy((returned + size - 1), " . ", 3);
		size = size + SPACE_DOT_SPACE;
		strncpy((returned + size - 1), lexeme_to_string(right), (extension + 1));
		size = size + extension;
	}

	returned = realloc(returned, (size + 2) * sizeof(char));
	strncpy((returned + size - 1), ")", 2);
	return returned;
}
static char * to_string_list(lexeme l) {
	char * returned = malloc(2 * sizeof(char));
	strncpy(returned, "(", 2);
	int size = 2; // null terminated strings
	int extension;
	const int COMMA_SPACE = 2; // space for a ", "
	while (l != &s_nil) {
		extension = strlen(lexeme_to_string(lexeme_get_left(l)));
		returned = realloc(returned, (size + extension + COMMA_SPACE + 1) * sizeof(char));
		strncpy((returned + size - 1), lexeme_to_string(lexeme_get_left(l)), (extension + 1));
		size = size + extension;
		l = l->right;
		if (l != &s_nil) {
			strncpy((returned + size - 1), ", ", 2);
			size = size + COMMA_SPACE;
		}
	}
	strncpy((returned + size - 1), ")", 2);
	return returned;
}

static char * to_string_type(lexeme l) {
	lexeme_type type = *((lexeme_type *) lexeme_get_data(l));
	return naturalNames[type];
}
