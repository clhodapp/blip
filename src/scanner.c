#include <stdlib.h>
#include <stdio.h>
#include <lex.h>
#include <lexeme.h>
#include <stddef.h>
#include <bigint.h>
#include <bigfloat.h>
#include <stddef.h>

void print_line(lexeme l);
void print_type(lexeme l);
void print_data(lexeme l);

int main(int argc, char** argv) {
	lex_stream l;
	if (argc < 2) {
		fprintf(stderr, "No filename given. Reading from stdin.\n");
		l = lex_stream_open_file(stdin);
	}
	else {
		l = lex_stream_open(argv[1]);
	}

	if (l == NULL) {
		fprintf(stderr, "Error opening file %s... exiting\n", argv[1]);
		exit(1);
	}
	lexeme lm;
	while (lexeme_get_type(lm = lex(l)) != END_OF_FILE) {
		print_line(lm);
		print_type(lm);
		printf(" ");
		print_data(lm);
		printf("\n");
		lexeme_destroy(lm);
	}
	print_line(lm);
	print_type(lm);
	printf(" ");
	print_data(lm);
	printf("\n");
	lexeme_destroy(lm);
	lex_stream_close(l);
	return 0;
}

void print_type(lexeme l) {
	char * typename = lexeme_get_typename(l);
	printf("%s", typename);
}

void print_data(lexeme l) {
	lexeme_type type = lexeme_get_type(l);
	if (type == STRING || type == ID) {
		printf("%s", (char *) lexeme_get_data(l));
	}
	else if (type == INT) {
		char * s = bitoa((bigint) lexeme_get_data(l));
		printf("%s", s);
		free(s);
	}
	else if (type == DEC) {
		char * s = bftoa((bigfloat) lexeme_get_data(l));
		printf("%s", s);
		free(s);
	}
}

void print_line(lexeme l) {
	char * linenum = bitoa(lexeme_get_linenum(l));
	printf("%s: ", linenum);
	free(linenum);
}
