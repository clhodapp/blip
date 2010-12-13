
#include <eval.h>
#include <lexeme.h>
#include <parser.h>
#include <lex.h>
#include <environment.h>
#include <prettyprinter.h>

int main(int argc, char **argv) {
	lex_stream source;
	if (argc < 2) {
		fprintf(stderr, "No filename given. Reading from stdin\n");
		source = lex_stream_open_file(stdin);
	}
	else {
		source = lex_stream_open(argv[1]);
	}

	lexeme tree = parse(source);
	lexeme env = env_make();
	eval_init(env);
	eval(env, tree);
	return 0;
}
