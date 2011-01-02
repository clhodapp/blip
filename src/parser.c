
#include <lex.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pair.h>

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
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	root = lexeme_make(PAIR);
	rootLeft =  primary();
	if (check(COMMA)) {
		lexeme_destroy(match(COMMA));
		rootRight = argList();
	}
	else {
		rootRight = lexeme_make(NIL);
	}
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
	return root;
}

static lexeme binding() {
	lexeme root;
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	root = match(BIND);
	lexeme_destroy(match(OPAREN));
	rootLeft = match(ID);
	lexeme_destroy(match(COMMA));
	rootRight = primary();
	lexeme_destroy(match(CPAREN));
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
	return root;
}

static lexeme importForm() {
	lexeme root;
	lexeme importedString;
	lexeme importedEnd;
	lexeme savedPending;
	lex_stream ls;
	pair p;
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
	while(lexeme_get_type(pair_get_right(lexeme_get_data(importedEnd))) != NIL) importedEnd = pair_get_right(lexeme_get_data(importedEnd));
	if (checkUnitList()) {
		p = pair_make(pair_get_left(lexeme_get_data(importedEnd)), unitList());
		lexeme_set_data(importedEnd, p);
	}
	return root;
}

static lexeme block() {
	lexeme root;
	lexeme rootLeft;
	lexeme rootRight = lexeme_make(NIL);
	pair rootPair;
	root = lexeme_make(BLOCK);
	lexeme_destroy(match(OBRACE));
	rootLeft = unitList();
	lexeme_destroy(match(CBRACE));
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
	return root;
}

static lexeme callable() {
	lexeme root;
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	lexeme calledLexeme = called();
	if (check(OPAREN)) {
		root = lexeme_make(CALL);
		rootLeft = calledLexeme;
		rootRight = lexeme_make(NIL);
		lexeme_destroy(match(OPAREN));
		if (checkArgList()) {
			lexeme_destroy(rootRight);
			rootRight = argList();
		}
		lexeme_destroy(match(CPAREN));
		rootPair = pair_make(rootLeft, rootRight);
		lexeme_set_data(root, rootPair);
	}
	else {
		root = calledLexeme;
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
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	root = match(LAMBDA);
	lexeme_destroy(match(OPAREN));
	lexeme_destroy(match(OPAREN));
	if (checkParamList()) {
		rootLeft = paramList();
	}
	else {
		rootLeft = lexeme_make(NIL);
	}
	lexeme_destroy(match(CPAREN));
	lexeme_destroy(match(COMMA));
	rootRight = primary();
	lexeme_destroy(match(CPAREN));
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
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
	lexeme rootLeft;
	lexeme rootRight = lexeme_make(NIL);
	pair rootPair;
	if (check(DOT)) {
		root = match(DOT);
		rootLeft = match(ID);
		rootPair = pair_make(rootLeft, rootRight);
		lexeme_set_data(root, rootPair);
	}
	else {
		root = match(ID);
	}

	return root;
}

static lexeme paramList() {
	lexeme root = lexeme_make(PAIR);
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	lexeme ampLeft;
	lexeme ampRight = lexeme_make(NIL);
	pair ampPair;
	if(check(AMP)) {
		rootLeft = match(AMP);
		ampLeft =  param();
		rootRight = lexeme_make(NIL);
		ampPair = pair_make(ampLeft, ampRight);
		lexeme_set_data(rootLeft, ampPair);
	}
	else {
		rootLeft = param();
		if (check(COMMA)) {
			lexeme_destroy(match(COMMA));
			rootRight = paramList();
		}
		else {
			rootRight = lexeme_make(NIL);
		}
	}
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
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
	lexeme rootLeft;
	lexeme rootRight = lexeme_make(NIL);
	pair rootPair;
	root = match(RETURN);
	lexeme_destroy(match(OPAREN));
	rootLeft = primary();
	lexeme_destroy(match(CPAREN));
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
	return root;
}

static lexeme unitList() {
	lexeme root;
	lexeme rootLeft;
	lexeme rootRight;
	pair rootPair;
	root = lexeme_make(UNITLIST);
	if (checkPrimary()) {
		rootLeft = primary();
	}
	else {
		rootLeft = returnForm();
	}
	lexeme_destroy(match(SEMI));
	if (checkUnitList()) {
		rootRight = unitList();
	}
	else if (checkImportForm()) {
		rootRight = importForm();
	}
	else {
		rootRight = lexeme_make(NIL);
	}
	rootPair = pair_make(rootLeft, rootRight);
	lexeme_set_data(root, rootPair);
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
