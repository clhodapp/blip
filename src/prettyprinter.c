#include <stdlib.h>
#include <lex.h>
#include <stddef.h>
#include <stdio.h>
#include <bigint.h>
#include <bigfloat.h>
#include <prettyprinter.h>
#include <pair.h>

void pretty_print(lexeme tree);

static void pretty_print_amp(lexeme tree);
static void pretty_print_dot(lexeme tree);
static void pretty_print_return(lexeme tree);
static void pretty_print_block(lexeme tree);
static void pretty_print_arglist(lexeme tree);
static void pretty_print_unitlist(lexeme tree);
static void pretty_print_bind(lexeme tree);
static void pretty_print_id(lexeme tree);
static void pretty_print_int(lexeme tree);
static void pretty_print_dec(lexeme tree);
static void pretty_print_nil(lexeme tree);
static void pretty_print_true(lexeme tree);
static void pretty_print_false(lexeme tree);
static void pretty_print_lambda(lexeme tree);
static void pretty_print_paramlist(lexeme tree);
static void pretty_print_call(lexeme tree);
static void pretty_print_string(lexeme tree);
static void pretty_print_pair(lexeme tree);

static void printtabs();
static lexeme get_left(lexeme);
static lexeme get_right(lexeme);

static unsigned int tabcount = 0;

void pretty_print(lexeme tree) {

	if (tree == NULL) {
		//fprintf(stderr, "Got empty tree. Returning...\n");
		return;
	}
	
	lexeme_type type = lexeme_get_type(tree);
	switch(type) {
		case PAIR:
			pretty_print_pair(tree);
			break;
		case STRING:
			pretty_print_string(tree);
			break;
		case AMP:
			pretty_print_amp(tree);
			break;
		case DOT:
			pretty_print_dot(tree);
			break;
		case RETURN:
			pretty_print_return(tree);
			break;
		case BLOCK:
			pretty_print_block(tree);
			break;
		case UNITLIST:
			pretty_print_unitlist(tree);
			break;
		case INT:
			pretty_print_int(tree);
			break;
		case DEC:
			pretty_print_dec(tree);
			break;
		case TRUE:
			pretty_print_true(tree);
			break;
		case FALSE:
			pretty_print_false(tree);
			break;
		case NIL:
			pretty_print_nil(tree);
			break;
		case ID:
			pretty_print_id(tree);
			break;
		case BIND:
			pretty_print_bind(tree);
			break;
		case LAMBDA:
			pretty_print_lambda(tree);
			break;
		case PARAMLIST:
			pretty_print_paramlist(tree);
			break;
		case CALL:
			pretty_print_call(tree);
			break;
		case ARGLIST:
			pretty_print_arglist(tree);
			break;
		default:
			printf("BAD LEXEME");
			lexeme_recursive_destroy(tree);
			break;
	}
}

static void pretty_print_unitlist(lexeme tree) {
	lexeme current = get_left(tree);
	lexeme next = get_right(tree);
	lexeme_destroy(tree);
	pretty_print(current);
	printf(";\n");
	if (lexeme_get_type(next) != NIL) {
		printtabs();
		pretty_print(next);
	}
}

static void pretty_print_pair(lexeme tree) {
	if (lexeme_get_type(tree) == NIL) {
		return;
	}

	lexeme right = get_right(tree);
	pretty_print(get_left(tree));
	if (lexeme_get_type(right) != NIL) {
		printf(", ");
		pretty_print(right);
	}
	//lexeme_destroy(tree);
}

static void pretty_print_bind(lexeme tree) {
	lexeme idPart = get_left(tree);
	lexeme bodyPart = get_right(tree);
	lexeme_destroy(tree);
	printf("bind(");
	pretty_print(idPart);
	printf(", ");
	pretty_print(bodyPart);
	printf(")");
}

static void pretty_print_int(lexeme tree) {
	bigint i = lexeme_get_data(tree);
	char * printed = bitoa(i);
	lexeme_destroy(tree);
	printf("%s", printed);
	free(printed);
}

static void pretty_print_dec(lexeme tree) {
	bigfloat f = lexeme_get_data(tree);
	char * printed = bftoa(f);
	lexeme_destroy(tree);
	printf("%s", printed);
	free(printed); }

static void pretty_print_nil(lexeme tree) {
	lexeme_destroy(tree);
	printf("#n");
}

static void pretty_print_true(lexeme tree) {
	lexeme_destroy(tree);
	printf("#t");
}

static void pretty_print_false(lexeme tree) {
	lexeme_destroy(tree);
	printf("#f");
}

static void pretty_print_id(lexeme tree) {
	char * printed = lexeme_get_data(tree);
	printf("%s", printed);
	lexeme_destroy(tree);
}

static void pretty_print_lambda(lexeme tree) {
	lexeme paramList = get_left(tree);
	lexeme body = get_right(tree);
	lexeme_destroy(tree);
	printf("lambda((");
	pretty_print(paramList);
	printf("), ");
	pretty_print(body);
	printf(")");
}

static void pretty_print_paramlist(lexeme tree) {
	lexeme currentParam = get_left(tree);
	lexeme next = get_right(tree);
	lexeme_destroy(tree);
	pretty_print(currentParam);
	if (next != NULL) {
		printf(", ");
		pretty_print(next);
	}
}

static void pretty_print_call(lexeme tree) {
	lexeme caller = get_left(tree);
	lexeme argList = get_right(tree);
	lexeme_destroy(tree);
	pretty_print(caller);
	printf("(");
	pretty_print(argList);
	printf(")");
}

static void pretty_print_arglist(lexeme tree) {
	lexeme currentArg = get_left(tree);
	lexeme next = get_right(tree);
	lexeme_destroy(tree);
	pretty_print(currentArg);
	if (next != NULL) {
		printf(", ");
		pretty_print(next);
	}
}

static void pretty_print_block(lexeme tree) {
	lexeme next = get_left(tree);
	lexeme_destroy(tree);
	printf("{\n");
	tabcount++;
	printtabs();
	if (next != NULL) {
		pretty_print(next);
	}
	tabcount--;
	printtabs();
	printf("}");
}

static void pretty_print_return(lexeme tree) {
	lexeme next = get_left(tree);
	lexeme_destroy(tree);
	printf("return(");
	pretty_print(next);
	printf(")");
}

static void pretty_print_dot(lexeme tree) {
	lexeme next = get_left(tree);
	lexeme_destroy(tree);
	printf(".");
	pretty_print(next);
}

static void pretty_print_amp(lexeme tree) {
	lexeme next = get_left(tree);
	lexeme_destroy(tree);
	printf("&");
	pretty_print(next);
}

static void pretty_print_string(lexeme tree) {
	char * printable = (char *) lexeme_get_data(tree);
	printf("\"%s\"", printable);
	lexeme_destroy(tree);
}

static void printtabs() {
	for(unsigned int i = tabcount; i > 0; i--) {
		printf("\t");
	}
}

static lexeme get_left(lexeme l) {
	pair p = (pair) lexeme_get_data(l);
	return pair_get_left(p);
}

static lexeme get_right(lexeme l) {
	pair p = (pair) lexeme_get_data(l);
	return pair_get_right(p);
}
