
#include <stdlib.h>

#include <lexeme.h>
#include <bigint.h>
#include <bigfloat.h>
#include <lexemetypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
	[TYPE] = "type"
};

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

static struct lexeme_t s_nil = {
	.type = NIL,
	.unique = true
};

static struct lexeme_t s_true = {
	.type = TRUE,
	.unique = true
};

static struct lexeme_t s_false = {
	.type = FALSE,
	.unique = true
};


lexeme NIL_LEXEME = &s_nil;
lexeme TRUE_LEXEME = &s_true;
lexeme FALSE_LEXEME = &s_false;

static char * (*stringifiers[LEXEME_TYPE_MAX + 1])(lexeme l) = {
	[INT] = &to_string_int,
	[STRING] = &to_string_string,
	[DEC] = &to_string_dec,
	[ID] = &to_string_id,
	[TRUE] = &to_string_true,
	[FALSE] = &to_string_false,
	[NIL] = &to_string_nil,
	[LIST] = &to_string_list,
	[TYPE] = &to_string_type
};

static void lexeme_dec_free_data(lexeme l);
static void lexeme_int_free_data(lexeme l);

lexeme lexeme_make(lexeme_type type) {
	lexeme l = (lexeme) malloc(sizeof(struct lexeme_t));
	l->type = type;
	l->data_initialized = false;
	l->left = NULL;
	l->right = NULL;
	l->unique = false;
	return l;
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
	if (!l->unique) {
		if (l->type == DEC) {
			lexeme_dec_free_data(l);
		}
		else if (l->type == INT) {
			lexeme_int_free_data(l);
		}
		if (l->data_initialized) {
			free(l->data);
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

static void lexeme_dec_free_data(lexeme l) {
	bigfloat f = (bigfloat) l->data;
	bigfloat_destroy(f);
	l->data_initialized = false;
}

static void lexeme_int_free_data(lexeme l) {
	bigint b = (bigint) l->data;
	bigint_destroy(b);
	l->data_initialized = false;
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

static char * to_string_list(lexeme l) {
	char * returned = malloc(2 * sizeof(char));
	strncpy(returned, "(", 2);
	int size = 2; // null terminated strings
	int extension;
	const int COMMA_SPACE = 2; // space for a ", "
	while (l != NIL_LEXEME) {
		extension = strlen(lexeme_to_string(lexeme_get_left(l)));
		returned = realloc(returned, (size + extension + COMMA_SPACE + 1) * sizeof(char));
		strncpy((returned + size - 1), lexeme_to_string(lexeme_get_left(l)), (extension + 1));
		size = size + extension;
		l = l->right;
		if (l != NIL_LEXEME) {
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
