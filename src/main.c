
#include <eval.h>
#include <lexeme.h>
#include <parser.h>
#include <lex.h>
#include <environment.h>
#include <prettyprinter.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static char * inPath = NULL;
static enum {
	INTERACTIVE_MODE,
	SCAN_MODE,
	CHECK_MODE,
	REFORMAT_MODE,
	STANDARD_MODE,
	HELP_MODE,
	ERROR_MODE
} mode;

char ** progArgs;

static void recognizeArguments(int argc, char * argv[]);
static void printHelp();
static void lex_loop(lex_stream source);
static void do_eval_loop(lexeme env);

static void recognizeArguments(int argc, char * argv[]) {
	mode = STANDARD_MODE;
	char * tmp;
	int i;

	for(i = 1; i < argc; i++) {
		tmp = argv[i];
		if (!strncmp(argv[i], "-i", 3) ||
				!strncmp(argv[i], "--interactive", 14)) {
			mode = INTERACTIVE_MODE;
		}
		else if (!strncmp(argv[i], "-s", 3) ||
				!strncmp(argv[i], "--scan", 6)) {
			mode = SCAN_MODE;
		}
		else if (!strncmp(argv[i], "-c", 3) ||
				!strncmp(argv[i], "--check", 8)) {
			mode = CHECK_MODE;
		}
		else if (!strncmp(argv[i], "-r", 3) ||
				!strncmp(argv[i], "--reformat", 11)) {
			mode = REFORMAT_MODE;
		}
		else if (!strncmp(argv[i], "-h", 3) ||
				!strncmp(argv[i], "--help", 7)) {
			mode = HELP_MODE;
		}
		else {
			inPath = argv[i]; 
			++i;
			break;
		}
	}
	progArgs = argv + i;
	return;
}

static void printHelp() {
	printf("Usage: blip [mode] file [args]...\n");
	printf("Mode:\n");
	printf("\t-i, --interactive\tRead-eval loop mode\n");
	printf("\t-s, --scan\tPrint lexical stream\n");
	printf("\t-c, --check\tCheck source syntax\n");
	printf("\t-r, --reformat\tReformat source to be easier to read\n");
	printf("\t-h, --help\tShow this help text\n");
}

static void lex_loop(lex_stream source) {
	lexeme pending;

	do {
		pending = lex(source);
		printf("%s: %s - %s\n", bitoa(lexeme_get_linenum(pending)),
				lexeme_get_typename(pending), lexeme_to_string(pending));
	} while (lexeme_get_type(pending) != END_OF_FILE);
}

static void do_eval_loop(lexeme env) {
	FILE * tmp;
	char input[1024];
	lexeme parseTree;
	lex_stream stream;
	do {
		tmp= tmpfile();
		printf(">>");
		fgets(input, 1024, stdin);
		fprintf(tmp, "%s", input);
		rewind(tmp);
		stream = lex_stream_open_file(tmp);
		parseTree = parse(stream);
		lex_stream_close(stream);
		eval(env, parseTree);
	} while (1);
}
		

int main(int argc, char *argv[]) {
	lex_stream source = NULL;
	lexeme env;
	recognizeArguments(argc, argv);

	if (mode == HELP_MODE) {
		printHelp();
		return 0;
	}

	env = env_make();
	eval_init(env);

	if (inPath != NULL) {
		source = lex_stream_open(inPath);
	}
	if (mode == SCAN_MODE) {
		lex_loop(source);
	}
	else if (source != NULL) {
		lexeme tree = parse(source);
		if (mode == CHECK_MODE) {
			return 0;
		}
		else if (mode == REFORMAT_MODE) {
			pretty_print(tree);
		}
		else {
			eval(env, tree);
		}
	}
	if (mode == INTERACTIVE_MODE) {
		do_eval_loop(env);
	}

	return 0;
}
