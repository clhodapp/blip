
#include <lex.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static lex_stream l;
static lexeme pending;

static lexeme advance();
static bool check(lexeme_type t);
static lexeme match(lexeme_type t);

static lexeme program();

static lexeme argList();
static lexeme binding();
static lexeme block();
static lexeme callable();
static lexeme called();
static lexeme lambdaForm();
static lexeme logic();
static lexeme numeric();
static lexeme param();
static lexeme paramList();
static lexeme primary();
static lexeme rawType();
static lexeme returnForm();
static lexeme importForm();
static lexeme unitList();

static bool checkArgList();
static bool checkBinding();
static bool checkBlock();
static bool checkCallable();
static bool checkCaller();
static bool checkLambdaForm();
static bool checkLogic();
static bool checkNumeric();
static bool checkParam();
static bool checkParamList();
static bool checkPrimary();
static bool checkRawType();
static bool checkReturnForm();
static bool checkUnitList();
static bool checkImportForm();

lexeme parse(lex_stream ls) {
	lexeme r;
	l = ls;
	advance();
	r = program();
	unlex(ls, pending);
	return r;
}

static lexeme program() {
	lexeme root;
	if (checkUnitList()) {
		root = unitList();
	}
	else {
		root = importForm();
	}
	return root;
}

static lexeme argList() {
	lexeme root;
	root = lexeme_make(LIST);
	lexeme_set_left(root, primary());
	if (check(COMMA)) {
		lexeme_destroy(match(COMMA));
		lexeme_set_right(root, argList());
	}
	else {
		lexeme_set_right(root, lexeme_make(NIL));
	}
	return root;
}

static lexeme binding() {
	lexeme root;
	root = match(BIND);
	lexeme_destroy(match(OPAREN));
	lexeme_set_left(root, match(ID));
	lexeme_destroy(match(COMMA));
	lexeme_set_right(root, primary());
	lexeme_destroy(match(CPAREN));
	return root;
}

static lexeme importForm() {
	lexeme root;
	lexeme importedString;
	lexeme importedEnd;
	lexeme savedPending;
	lex_stream ls;
	lexeme_destroy(match(IMPORT));
	importedString = match(STRING);
	lexeme_destroy(match(SEMI));
	lex_stream savedStream = l;
	ls = lex_stream_open((char *) lexeme_get_data(importedString));
	savedPending = pending;
	root = parse(ls);
	lexeme_destroy(importedString);
	lex_stream_close(ls);
	pending = savedPending;
	l = savedStream;

	importedEnd = root;
	while(lexeme_get_right(importedEnd) != NULL) importedEnd = lexeme_get_right(importedEnd);
	if (checkUnitList()) {
		lexeme_set_right(importedEnd, unitList());
	}
	else {
		lexeme_set_right(importedEnd, NULL);
	}
	return root;
}

static lexeme block() {
	lexeme root;
	root = lexeme_make(BLOCK);
	lexeme_destroy(match(OBRACE));
	lexeme_set_left(root, unitList());
	lexeme_destroy(match(CBRACE));
	return root;
}

static lexeme callable() {
	lexeme root = called();
	if (check(OPAREN)) {
		lexeme call = lexeme_make(CALL);
		lexeme_set_left(call, root);
		lexeme_set_right(call, lexeme_make(NIL));
		root = call;
		lexeme_destroy(match(OPAREN));
		if (checkArgList()) {
			lexeme_set_right(root, argList());
		}
		lexeme_destroy(match(CPAREN));
	}
	return root;

}

static lexeme called() {
	lexeme root;
	if (check(ID)) {
		root = match(ID);
	}
	else if (checkBlock()) {
		root = block();
	}
	else if (checkLambdaForm()) {
		root = lambdaForm();
	}
//	else if (checkReturnForm()) {
//		root = returnForm();
//	}
	else {
		root = binding();
	}
	return root;
}



static lexeme lambdaForm() {
	lexeme root;
	root = match(LAMBDA);
	lexeme_destroy(match(OPAREN));
	lexeme_destroy(match(OPAREN));
	if (checkParamList()) {
		lexeme_set_left(root, paramList());
	}
	else {
		lexeme_set_left(root, lexeme_make(NIL));
	}
	lexeme_destroy(match(CPAREN));
	lexeme_destroy(match(COMMA));
	lexeme_set_right(root, primary());
	lexeme_destroy(match(CPAREN));
	return root;
}

static lexeme logic() {
	lexeme root;
	if (check(TRUE)) {
		root = match(TRUE);
	}
	else if(check(FALSE)) {
		root = match(FALSE);
	}
	else {
		root = match(NIL);
	}
	return root;
}

static lexeme numeric() {
	lexeme root;
	if (check(INT)) {
		root = match(INT);
	}
	else {
		root = match(DEC);
	}
	return root;
}

static lexeme param() {
	lexeme root;
	root = NULL;
	if (check(DOT)) {
		root = match(DOT);
	}
	lexeme id = match(ID);
	if (root) {
		lexeme_set_left(root, id);
	}
	else {
		root = id;
	}
	return root;
}

static lexeme paramList() {
	lexeme root = lexeme_make(LIST);
	lexeme amp;
	if(check(AMP)) {
		amp = match(AMP);
		lexeme_set_left(root, amp);
		lexeme_set_left(amp, param());
		lexeme_set_right(root, lexeme_make(NIL));
	}
	else {
		lexeme_set_left(root, param());
		if (check(COMMA)) {
			lexeme_destroy(match(COMMA));
			lexeme_set_right(root, paramList());
		}
		else {
			lexeme_set_right(root, lexeme_make(NIL));
		}
	}
	return root;
}

static lexeme primary() {
	lexeme root;
	if (checkRawType()) {
		root = rawType();
	}
	else {
		root = callable();
	}
	return root;
}

static lexeme rawType() {
	lexeme root;
	if (checkNumeric()) {
		root = numeric();
	}
	else if (check(STRING)){
		root = match(STRING);
	}
	else {
		root = logic();
	}
	return root;
}

static lexeme returnForm() {
	lexeme root;
	root = match(RETURN);
	lexeme_destroy(match(OPAREN));
	lexeme_set_left(root, primary());
	lexeme_destroy(match(CPAREN));
	return root;
}

static lexeme unitList() {
	lexeme root;
	root = lexeme_make(UNITLIST);
	if (checkPrimary()) {
		lexeme_set_left(root, primary());
	}
	else {
		lexeme_set_left(root, returnForm());
	}
	lexeme_destroy(match(SEMI));
	if (checkUnitList()) {
		lexeme_set_right(root, unitList());
	}
	else if (checkImportForm()) {
		lexeme_set_right(root, importForm());
	}
	else {
		lexeme_set_right(root, NULL);
	}
	return root;
}

static bool checkArgList() {
	return (checkPrimary());
}

static bool checkBinding() {
	return (check(BIND));
}

static bool checkBlock() {
	return (check(OBRACE));
}

static bool checkCallable() {
	return (checkCaller());
}

static bool checkCaller() {
	return (check(ID) || checkBlock() || checkLambdaForm() || checkBinding());
}

static bool checkLambdaForm() {
	return (check(LAMBDA));
}

static bool checkLogic() {
	return (check(TRUE) || check(FALSE) || check(NIL));
}

static bool checkNumeric() {
	return (check(INT) || check(DEC));
}

static bool checkParam() {
	return (check(ID) || check(DOT));
}

static bool checkParamList() {
	return (checkParam() || check(AMP));
}

static bool checkPrimary() {
	return (checkRawType() || checkCallable());
}

static bool checkRawType() {
	return (checkNumeric() || check(STRING) || checkLogic());
}

static bool checkReturnForm() {
	return (check(RETURN));
}

static bool checkImportForm() {
	return (check(IMPORT));
}

static bool checkUnitList() {
	return (checkPrimary() || checkReturnForm());
}


static lexeme advance() {
	lexeme temp = pending;
	pending = lex(l);
	return temp;
}

static bool check(lexeme_type t) {
	return (lexeme_get_type(pending) == t);
}

static lexeme match(lexeme_type t) {
	if (lexeme_get_type(pending) == t) {
		return advance();
	}
	else {
		char * line = bitoa(lexeme_get_linenum(pending));
		fprintf(stdout, "Error on line %s: Expected lexeme of type %s, but got %s\n", line, lexeme_type_to_string(t), lexeme_type_to_string(lexeme_get_type(pending)));
		free(line);
		exit(1);
	}
}
