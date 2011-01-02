
#include <builtins.h>
#include <lexeme.h>
#include <environment.h>
#include <bigint.h>
#include <bigfloat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <eval.h>
#include <string.h>
#include <pair.h>

extern lexeme func_obj_make(lexeme env, lexeme params, lexeme body);

static lexeme create_call_return(lexeme evaled);

static lexeme out(lexeme args);
static lexeme err(lexeme args);
static lexeme plus(lexeme args);
static lexeme minus(lexeme args);
static lexeme multiply(lexeme args);
static lexeme divide(lexeme args);
static lexeme greater(lexeme args);
static lexeme list(lexeme args);
static lexeme less(lexeme args);
static lexeme nil(lexeme args);
static lexeme equal(lexeme args);
static lexeme head(lexeme args);
static lexeme tail(lexeme args);
static lexeme type(lexeme args);
static lexeme ifFunc(lexeme args);
static lexeme apply(lexeme args);
static lexeme cons(lexeme args);
static lexeme str_ref(lexeme args);
static lexeme append(lexeme args);
static lexeme apply_call(lexeme operator, lexeme callArgList);
static void register_builtin(lexeme env, char * name, lexeme (*action)(lexeme), lexeme paramList);
static lexeme build_list_list();
static lexeme build_type_list();
static lexeme build_head_list();
static lexeme build_sref_list();
static lexeme build_tail_list();
static lexeme build_apply_list();
static lexeme build_less_list();
static lexeme build_greater_list();
static lexeme build_out_list();
static lexeme build_err_list();
static lexeme build_plus_list();
static lexeme build_minus_list();
static lexeme build_div_list();
static lexeme build_times_list();
static lexeme build_equal_list();
static lexeme build_generic_list();
static lexeme build_out_list();
static lexeme build_nil_list();
static lexeme build_plus_list();
static lexeme build_if_list();
static lexeme build_equal_list();
static lexeme build_append_list();
static lexeme build_cons_list();
static lexeme build_generic_list(char * name);

static lexeme addInt(lexeme addend1, lexeme addend2);
static lexeme addDec(lexeme addend1, lexeme addend2);

static lexeme subInt(lexeme addend1, lexeme addend2);
static lexeme subDec(lexeme addend1, lexeme addend2);

static lexeme mulInt(lexeme multiplicand1, lexeme multiplicand2);
static lexeme mulDec(lexeme multiplicand1, lexeme multiplicand2);

static lexeme divInt(lexeme dividend, lexeme divisor);
static lexeme divDec(lexeme dividend, lexeme divisor);

static lexeme greaterInt(lexeme x1, lexeme x2);
static lexeme greaterDec(lexeme x1, lexeme x2);
static lexeme greaterDefault(lexeme x1, lexeme x2);

static lexeme lessInt(lexeme x1, lexeme x2);
static lexeme lessDec(lexeme x1, lexeme x2);
static lexeme lessDefault(lexeme x1, lexeme x2);

static bool equalInt(lexeme arg1, lexeme arg2);
static bool equalDefault(lexeme arg1, lexeme arg2);
static bool equalString(lexeme arg1, lexeme arg2);

lexeme (* addOps [LEXEME_TYPE_MAX + 1])(lexeme addend1, lexeme addend2) = {
	[INT] = &addInt,
	[DEC] = &addDec
};

lexeme (* subOps [LEXEME_TYPE_MAX + 1])(lexeme addend1, lexeme addend2) = {
	[INT] = &subInt,
	[DEC] = &subDec
};

lexeme (* mulOps [LEXEME_TYPE_MAX + 1])(lexeme multiplicand1, lexeme multiplicand2) = {
	[INT] = &mulInt,
	[DEC] = &mulDec
};

lexeme (* divOps [LEXEME_TYPE_MAX + 1])(lexeme multiplicand1, lexeme multiplicand2) = {
	[INT] = &divInt,
	[DEC] = &divDec
};

lexeme (* greaterOps [LEXEME_TYPE_MAX + 1])(lexeme x1, lexeme x2) = {
	[INT] = &greaterInt,
	[DEC] = &greaterDec
};

lexeme (* lessOps [LEXEME_TYPE_MAX + 1])(lexeme x1, lexeme x2) = {
	[INT] = &lessInt,
	[DEC] = &lessDec
};

bool (* equalOps [LEXEME_TYPE_MAX + 1])(lexeme arg1, lexeme arg2) = {
	[INT] = &equalInt,
	[STRING] = &equalString
};

void register_builtins(lexeme env) {
	lexeme outList = build_out_list();
	lexeme errList = build_err_list();
	lexeme plusList = build_plus_list();
	lexeme minusList = build_minus_list();
	lexeme timesList = build_times_list();
	lexeme divList = build_div_list();
	lexeme greaterList = build_greater_list();
	lexeme lessList = build_less_list();
	lexeme equalList = build_equal_list();
	lexeme listList = build_list_list();
	lexeme ifList = build_if_list();
	lexeme headList = build_head_list();
	lexeme tailList = build_tail_list();
	lexeme nilList = build_nil_list();
	lexeme applyList = build_apply_list();
	lexeme typeList = build_type_list();
	lexeme consList = build_cons_list();
	lexeme appendList = build_append_list();
	lexeme srefList = build_sref_list();
	register_builtin(env, "out", &out, outList);
	register_builtin(env, "err", &err, errList);
	register_builtin(env, "+", &plus, plusList);
	register_builtin(env, "-", &minus, minusList);
	register_builtin(env, "*", &multiply, timesList);
	register_builtin(env, "/", &divide, divList);
	register_builtin(env, "greater?", &greater, greaterList);
	register_builtin(env, "less?", &less, lessList);
	register_builtin(env, "list", &list, listList);
	register_builtin(env, "equal?", &equal, equalList);
	register_builtin(env, "if", &ifFunc, ifList);
	register_builtin(env, "head", &head, headList);
	register_builtin(env, "tail", &tail, tailList);
	register_builtin(env, "apply", &apply, applyList);
	register_builtin(env, "nil?", &nil, nilList);
	register_builtin(env, "type", &type, typeList);
	register_builtin(env, "cons", &cons, consList);
	register_builtin(env, "append", &append, appendList);
	register_builtin(env, "sref", &str_ref, srefList);
}

lexeme (*builtin_get_action(lexeme b))(lexeme l) {
	return lexeme_get_data(pair_get_left(lexeme_get_data(b)));
}

lexeme builtin_get_params(lexeme b) {
	return pair_get_right(lexeme_get_data(b));
}

static void register_builtin(lexeme env, char * name, lexeme (*action)(lexeme), lexeme paramList) {
	lexeme actionLexeme = lexeme_make(ACTION);
	lexeme id = lexeme_make(ID);
	lexeme registered = lexeme_make(BUILTIN);
	lexeme_set_data(actionLexeme, action);
	pair p = pair_make(actionLexeme, paramList);
	lexeme_set_data(id, name);
	lexeme_set_data(registered, p);
	env_insert(env, id, registered);
}

static lexeme out(lexeme args) {
	args = pair_get_left(lexeme_get_data(args));
	while (args != lexeme_make(NIL)) {
		printf("%s", lexeme_to_string(pair_get_left(lexeme_get_data(args))));
		args = pair_get_right(lexeme_get_data(args));
	}
	return lexeme_make(NIL);
}

static lexeme str_ref(lexeme args) {
	lexeme strLexeme = pair_get_left(lexeme_get_data(args));
	long long unsigned int index = ((bigint) lexeme_get_data(pair_get_left(lexeme_get_data((pair_get_right(lexeme_get_data(args)))))))->data;
	char * str = lexeme_get_data(strLexeme);
	size_t i = (size_t) index;
	lexeme r = lexeme_make(STRING);
	char * c = malloc(2 * sizeof(char));
	c[0] = str[i];
	c[1] = 0;
	lexeme_set_data(r, c);
	return r;
}

static lexeme err(lexeme args) {
	args = pair_get_left(lexeme_get_data(args));
	while (args != lexeme_make(NIL)) {
		fprintf(stderr, "%s", lexeme_to_string(pair_get_left(lexeme_get_data(args))));
		args = pair_get_right(lexeme_get_data(args));
	}
	return lexeme_make(NIL);
}

static lexeme list(lexeme args) {
	return pair_get_left(lexeme_get_data(args));
}

static lexeme head(lexeme args) {
	return pair_get_left(lexeme_get_data(args));
}

static lexeme tail(lexeme args) {
	return pair_get_right(lexeme_get_data(pair_get_left(lexeme_get_data(args))));
}

static lexeme cons(lexeme args) {
	lexeme returned = lexeme_make(PAIR);
	lexeme head = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	lexeme tail = pair_get_left(lexeme_get_data(args));
	pair p = pair_make(head, tail);
	lexeme_set_data(returned, p);
	return returned;
}


static lexeme plus(lexeme args) {
	lexeme (*addOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;
	pair p;

	args = pair_get_left(lexeme_get_data(args));

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: + requires at least one argument");
	}
	addOp = addOps[lexeme_get_type(pair_get_left(lexeme_get_data(args)))];
	if (addOp == NULL) {
		fprintf(stderr, "ERROR: Unaddable type");
		exit(EXIT_FAILURE);
	}

	while (pair_get_right(lexeme_get_data(args)) != lexeme_make(NIL)) {
		current = pair_get_left(lexeme_get_data(args));
		right = pair_get_right(lexeme_get_data(args));
		next = pair_get_left(lexeme_get_data(right));
		new = lexeme_make(ARGLIST);
		p = pair_make(addOp(current, next), pair_get_right(lexeme_get_data(right)));
		lexeme_set_data(new, p);
		args = new;
	}
	return pair_get_left(lexeme_get_data(args));
}

static lexeme minus(lexeme args) {
	lexeme (*subOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;
	pair p;

	args = pair_get_left(lexeme_get_data(args));

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: - requires at least one argument");
	}
	subOp = subOps[lexeme_get_type(pair_get_left(lexeme_get_data(args)))];
	if (subOp == NULL) {
		fprintf(stderr, "ERROR: Unsubtractable type");
		exit(EXIT_FAILURE);
	}

	while (pair_get_right(lexeme_get_data(args)) != lexeme_make(NIL)) {
		current = pair_get_left(lexeme_get_data(args));
		right = pair_get_right(lexeme_get_data(args));
		next = pair_get_left(lexeme_get_data(right));
		new = lexeme_make(ARGLIST);
		p = pair_make(subOp(current, next), pair_get_right(lexeme_get_data(right)));
		lexeme_set_data(new, p);
		args = new;
	}
	return pair_get_left(lexeme_get_data(args));
}

static lexeme multiply(lexeme args) {
	lexeme (*mulOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;
	pair p;

	args = pair_get_left(lexeme_get_data(args));

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: * requires at least one argument");
	}
	mulOp = mulOps[lexeme_get_type(pair_get_left(lexeme_get_data(args)))];
	if (mulOp == NULL) {
		fprintf(stderr, "ERROR: Unmultipliable type");
		exit(EXIT_FAILURE);
	}

	while (pair_get_right(lexeme_get_data(args)) != lexeme_make(NIL)) {
		current = pair_get_left(lexeme_get_data(args));
		right = pair_get_right(lexeme_get_data(args));
		next = pair_get_left(lexeme_get_data(right));
		new = lexeme_make(ARGLIST);
		p = pair_make(mulOp(current, next), pair_get_right(lexeme_get_data(right)));
		lexeme_set_data(new, p);
		args = new;
	}
	return pair_get_left(lexeme_get_data(args));
}

static lexeme divide(lexeme args) {
	lexeme (*divOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;
	pair p;

	args = pair_get_left(lexeme_get_data(args));

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: / requires at least one argument");
	}
	divOp = divOps[lexeme_get_type(pair_get_left(lexeme_get_data(args)))];
	if (divOp == NULL) {
		fprintf(stderr, "ERROR: Undividable type");
		exit(EXIT_FAILURE);
	}

	while (pair_get_right(lexeme_get_data(args)) != lexeme_make(NIL)) {
		current = pair_get_left(lexeme_get_data(args));
		right = pair_get_right(lexeme_get_data(args));
		next = pair_get_left(lexeme_get_data(right));
		new = lexeme_make(ARGLIST);
		p = pair_make(divOp(current, next), pair_get_right(lexeme_get_data(right)));
		lexeme_set_data(new, p);
		args = new;
	}
	return pair_get_left(lexeme_get_data(args));
}

lexeme append(lexeme args) {
	lexeme first = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	lexeme second = pair_get_left(lexeme_get_data(args));
	lexeme retFirst;
	lexeme retSecond = lexeme_make(NIL);
	lexeme currentItem;
	pair p;
	if (second != lexeme_make(NIL)) {
		currentItem = lexeme_make(PAIR);
		retSecond = currentItem;
		p = pair_make(pair_get_left(lexeme_get_data(second)), lexeme_make(NIL));
		lexeme_set_data(currentItem, p);
		while (pair_get_right(lexeme_get_data(second)) != lexeme_make(NIL)) {
			second = pair_get_right(lexeme_get_data(second));
			p = pair_make(pair_get_left(lexeme_get_data(currentItem)), lexeme_make(PAIR));
			lexeme_set_data(currentItem, p);
			currentItem = pair_get_right(p);
			p = pair_make(pair_get_left(lexeme_get_data(second)), lexeme_make(NIL));
			lexeme_set_data(currentItem, p);
		}
	}
	// else,retSecond remains NIL
	if (first != lexeme_make(NIL)) {
		currentItem = lexeme_make(PAIR);
		retFirst = currentItem;
		p = pair_make(pair_get_left(lexeme_get_data(first)), retSecond);
		lexeme_set_data(currentItem, p);
		while (pair_get_right(lexeme_get_data(first)) != lexeme_make(NIL)) {
			first = pair_get_right(lexeme_get_data(first));
			p = pair_make(pair_get_left(lexeme_get_data(currentItem)), lexeme_make(PAIR));
			lexeme_set_data(currentItem, p);
			currentItem = pair_get_right(p);
			p = pair_make(pair_get_left(lexeme_get_data(first)), retSecond);
		}
	}
	else {
		retFirst = retSecond;
	}
	return retFirst;
}

lexeme nil(lexeme args) {
	return lexeme_make(pair_get_left(lexeme_get_data(args)) == lexeme_make(NIL) ? TRUE : FALSE);
}

lexeme greater(lexeme args) {
	lexeme x1 = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	lexeme x2 = pair_get_left(lexeme_get_data(args));
	lexeme_type type = lexeme_get_type(x1);
	lexeme (*greaterOp)(lexeme arg1, lexeme arg2);
	if (lexeme_get_type(x2) != type) {
		fprintf(stderr, "Error -- greater? on line %s: type mismatch -- %s and %s", bitoa(lexeme_get_data(x2)), lexeme_get_typename(x1), lexeme_get_typename(x2));
		exit(EXIT_FAILURE);
	}
	greaterOp = greaterOps[type];
	if (greaterOp == NULL) {
		greaterOp = &greaterDefault;
	}
	return greaterOp(x1, x2);
}

lexeme less(lexeme args) {
	lexeme x1 = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	lexeme x2 = pair_get_left(lexeme_get_data(args));
	lexeme_type type = lexeme_get_type(x1);
	lexeme (*lessOp)(lexeme arg1, lexeme arg2);
	if (lexeme_get_type(x2) != type) {
		fprintf(stderr, "Error -- less? on line %s: type mismatch", bitoa(lexeme_get_data(x2)));
		exit(EXIT_FAILURE);
	}
	lessOp = lessOps[type];
	if (lessOp == NULL) {
		lessOp = &lessDefault;
	}
	return lessOp(x1, x2);
}

static lexeme addInt(lexeme addend1, lexeme addend2) {
	lexeme new = lexeme_make(INT);
	bigint val1 = lexeme_get_data(addend1);
	bigint val2 = lexeme_get_data(addend2);
	lexeme_set_data(new, bigint_add(val1, val2));
	return new;
}

static lexeme addDec(lexeme addend1, lexeme addend2) {
	lexeme new = lexeme_make(DEC);
	bigfloat val1 = lexeme_get_data(addend1);
	bigfloat val2 = lexeme_get_data(addend2);
	lexeme_set_data(new, bigfloat_add(val1, val2));
	return new;
}

static lexeme subInt(lexeme addend1, lexeme addend2) {
	lexeme new = lexeme_make(INT);
	bigint val1 = lexeme_get_data(addend1);
	bigint val2 = lexeme_get_data(addend2);
	lexeme_set_data(new, bigint_sub(val1, val2));
	return new;
}

static lexeme subDec(lexeme addend1, lexeme addend2) {
	lexeme new = lexeme_make(DEC);
	bigfloat val1 = lexeme_get_data(addend1);
	bigfloat val2 = lexeme_get_data(addend2);
	lexeme_set_data(new, bigfloat_sub(val1, val2));
	return new;
}

static lexeme mulInt(lexeme multiplicand1, lexeme multiplicand2) {
	lexeme new = lexeme_make(INT);
	bigint val1 = lexeme_get_data(multiplicand1);
	bigint val2 = lexeme_get_data(multiplicand2);
	lexeme_set_data(new, bigint_mult(val1, val2));
	return new;
}

static lexeme mulDec(lexeme multiplicand1, lexeme multiplicand2) {
	lexeme new = lexeme_make(DEC);
	bigfloat val1 = lexeme_get_data(multiplicand1);
	bigfloat val2 = lexeme_get_data(multiplicand2);
	lexeme_set_data(new, bigfloat_mult(val1, val2));
	return new;
}

static lexeme divInt(lexeme dividend, lexeme divisor) {
	lexeme new = lexeme_make(INT);
	bigint val1 = lexeme_get_data(dividend);
	bigint val2 = lexeme_get_data(divisor);
	lexeme_set_data(new, bigint_div(val1, val2));
	return new;
}

static lexeme divDec(lexeme dividend, lexeme divisor) {
	lexeme new = lexeme_make(DEC);
	bigfloat val1 = lexeme_get_data(dividend);
	bigfloat val2 = lexeme_get_data(divisor);
	lexeme_set_data(new, bigfloat_div(val1, val2));
	return new;
}

static lexeme greaterInt(lexeme x1, lexeme x2) {
	return lexeme_make(bigint_greater(lexeme_get_data(x1),lexeme_get_data(x2)) ?
			TRUE : FALSE);
}

static lexeme greaterDec(lexeme x1, lexeme x2) {
	return lexeme_make(bigfloat_greater(lexeme_get_data(x1),lexeme_get_data(x2)) ?
			TRUE : FALSE);
}

static lexeme greaterDefault(lexeme x1, lexeme x2) {
	return lexeme_make((x1 > x2) ?
			TRUE : FALSE);
}

static lexeme lessInt(lexeme x1, lexeme x2) {
	return lexeme_make(bigint_less(lexeme_get_data(x1),lexeme_get_data(x2)) ?
			TRUE : FALSE);
}

static lexeme lessDec(lexeme x1, lexeme x2) {
	return lexeme_make(bigfloat_less(lexeme_get_data(x1),lexeme_get_data(x2)) ?
			TRUE : FALSE);
}

static lexeme lessDefault(lexeme x1, lexeme x2) {
	return lexeme_make((x1 > x2) ?
			TRUE : FALSE);
}


static lexeme type(lexeme args) {
	lexeme returned = lexeme_make(TYPE);
	lexeme_type *type = malloc(sizeof(lexeme_type));
	*type = lexeme_get_type(pair_get_left(lexeme_get_data(args)));
	lexeme_set_data(returned, type);
	return returned;
}


static lexeme equal(lexeme args) {
	bool (* equalOp)(lexeme arg1, lexeme arg2);
	args = pair_get_left(lexeme_get_data(args));
	lexeme currentVal;
	lexeme nextVal;

	if (args == lexeme_make(NIL) || pair_get_right(lexeme_get_data(args)) == lexeme_make(NIL)) {
		fprintf(stderr, "ERROR -- EQUAL: Requires at least two arguments");
		exit(EXIT_FAILURE);
	}
	equalOp = equalOps[lexeme_get_type(pair_get_left(lexeme_get_data(args)))];

	if (equalOp == NULL) {
		equalOp = &equalDefault;
	}

	while (pair_get_right(lexeme_get_data(args)) != lexeme_make(NIL)) {
		currentVal = pair_get_left(lexeme_get_data(args));
		nextVal = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
		if (!equalOp(currentVal, nextVal)) {
			return lexeme_make(FALSE);
		}
		args = pair_get_right(lexeme_get_data(args));
	}
	return lexeme_make(TRUE);
}

static bool equalInt(lexeme arg1, lexeme arg2) {
	bigint val1 = lexeme_get_data(arg1);
	bigint val2 = lexeme_get_data(arg2);

	return bigint_equal(val1, val2);
}

static bool equalString(lexeme arg1, lexeme arg2) {
	char * val1 = lexeme_get_data(arg1);
	char * val2 = lexeme_get_data(arg2);
	return (strcmp(val1, val2) == 0);
}

static lexeme ifFunc(lexeme args) {
	lexeme ifFalse = pair_get_left(lexeme_get_data(args));
	lexeme ifTrue = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	lexeme conditional = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(pair_get_right(lexeme_get_data(args))))));
	return create_call_return(lexeme_get_type(conditional) == TRUE ? ifTrue : ifFalse);
}

static lexeme apply(lexeme args) {
	lexeme callArgList = lexeme_make(NIL);
	lexeme currentArg;
	lexeme newArg;
	pair p;
	lexeme operator = pair_get_left(lexeme_get_data(pair_get_right(lexeme_get_data(args))));
	args = pair_get_left(lexeme_get_data(args));


	if (args != lexeme_make(NIL)) {
		callArgList = lexeme_make(ARGLIST);
		p = pair_make(pair_get_left(lexeme_get_data(args)), lexeme_make(NIL));
		lexeme_set_data(callArgList, p);
		currentArg = callArgList;
		args = pair_get_right(lexeme_get_data(args));
	}
	while (args != lexeme_make(NIL)) {
		newArg = lexeme_make(ARGLIST);
		p = pair_make(pair_get_left(lexeme_get_data(args)), lexeme_make(NIL));
		lexeme_set_data(newArg, p);
		p = pair_make(pair_get_left(lexeme_get_data(currentArg)), newArg);
		lexeme_set_data(currentArg, p);
		currentArg = newArg;
		args = pair_get_right(lexeme_get_data(args));
	}

	return apply_call(operator, callArgList);

}

static lexeme apply_call(lexeme operator, lexeme callArgList) {
	lexeme callLexeme = lexeme_make(CALL);
	pair p = pair_make(operator, callArgList);
	lexeme_set_data(callLexeme, p);
	return callLexeme;
}

static bool equalDefault(lexeme arg1, lexeme arg2) {
	return arg1==arg2;
}


static lexeme build_out_list() {
	return build_generic_list("printed");
}

static lexeme build_err_list() {
	return build_generic_list("printed");
}

static lexeme build_plus_list() {
	return build_generic_list("added");
}

static lexeme build_minus_list() {
	return build_generic_list("subtracted");
}

static lexeme build_times_list() {
	return build_generic_list("multiplied");
}

static lexeme build_div_list() {
	return build_generic_list("divided");
}

static lexeme build_equal_list() {
	return build_generic_list("listed");
}

static lexeme build_if_list() {
	lexeme newParamList;
	lexeme currentParamList;
	lexeme dot;
	lexeme newParam;
	pair p;

	newParamList = lexeme_make(PAIR);
	dot = lexeme_make(DOT);
	newParam = lexeme_make(ID);
	p = pair_make(dot, lexeme_make(NIL));
	lexeme_set_data(newParam, p);
	p = pair_make(dot, lexeme_make(NIL));
	lexeme_set_data(dot, p);
	lexeme_set_data(newParam, "ifFalse");

	currentParamList = newParamList;

	newParamList = lexeme_make(PAIR);
	dot = lexeme_make(DOT);
	newParam = lexeme_make(ID);
	p = pair_make(dot, currentParamList);
	lexeme_set_data(newParamList,  p);
	
	p = pair_make(newParam, lexeme_make(NIL));
	lexeme_set_data(dot, p);
	lexeme_set_data(newParam, "ifTrue");

	currentParamList = newParamList;

	newParamList = lexeme_make(PAIR);
	newParam = lexeme_make(ID);
	p = pair_make(newParam, currentParamList);
	lexeme_set_data(newParamList, p);
	lexeme_set_data(newParam, "condition");

	return newParamList;
}

static lexeme build_less_list() {
	lexeme newParamList;
	lexeme currentParamList;
	lexeme newParam;
	pair p;

	newParamList = lexeme_make(PAIR);
	newParam = lexeme_make(ID);
	p = pair_make(newParam, lexeme_make(NIL));
	lexeme_set_data(newParamList, p);
	lexeme_set_data(newParam, "x1");
	
	currentParamList = newParamList;

	newParamList = lexeme_make(PAIR);
	newParam = lexeme_make(ID);
	p = pair_make(currentParamList, newParam);
	lexeme_set_data(newParamList, p);
	lexeme_set_data(newParam, "x2");

	return newParamList;
}

static lexeme build_greater_list() {
	lexeme newParamList;
	lexeme currentParamList;
	lexeme newParam;
	pair p;

	newParamList = lexeme_make(PAIR);
	newParam = lexeme_make(ID);
	p = pair_make(newParam, lexeme_make(NIL));
	lexeme_set_data(newParamList, p);
	lexeme_set_data(newParam, "x1");
	
	currentParamList = newParamList;

	newParamList = lexeme_make(PAIR);
	newParam = lexeme_make(ID);
	p = pair_make(currentParamList, newParam);
	lexeme_set_data(newParamList, p);
	lexeme_set_data(newParam, "x2");

	return newParamList;
}

static lexeme build_list_list() {
	return build_generic_list("listed");
}

static lexeme build_head_list() {
	lexeme paramList = lexeme_make(PAIR);
	lexeme param = lexeme_make(ID);
	pair p = pair_make(param, lexeme_make(NIL));
	lexeme_set_data(paramList, p);
	lexeme_set_data(param, "list");
	return paramList;
}

static lexeme build_tail_list() {
	lexeme paramList = lexeme_make(PAIR);
	lexeme param = lexeme_make(ID);
	pair p = pair_make(param, lexeme_make(NIL));
	lexeme_set_data(paramList, p);
	lexeme_set_data(param, "list");
	return paramList;
}

static lexeme build_nil_list() {
	lexeme paramList = lexeme_make(PAIR);
	lexeme param = lexeme_make(ID);
	pair p = pair_make(param, lexeme_make(NIL));
	lexeme_set_data(paramList, p);
	lexeme_set_data(param, "tested");
	return paramList;
}

static lexeme build_apply_list() {
	lexeme retParamList = lexeme_make(PAIR);
	lexeme rightParamList = lexeme_make(PAIR);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	pair p = pair_make(firstParam, rightParamList);
	lexeme_set_data(retParamList, p);
	p = pair_make(secondParam, lexeme_make(NIL));
	lexeme_set_data(rightParamList, p);
	lexeme_set_data(firstParam, "operator");
	lexeme_set_data(secondParam, "operands");
	return retParamList;
}

static lexeme build_type_list() {
	lexeme paramList = lexeme_make(PAIR);
	lexeme param = lexeme_make(ID);
	pair p = pair_make(param, lexeme_make(NIL));
	lexeme_set_data(paramList, p);
	lexeme_set_data(param, "arg");
	return paramList;
}

static lexeme build_cons_list() {
	lexeme retParamList = lexeme_make(PAIR);
	lexeme rightParamList = lexeme_make(PAIR);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	pair p = pair_make(firstParam, rightParamList);
	lexeme_set_data(retParamList, p);
	p = pair_make(secondParam, lexeme_make(NIL));
	lexeme_set_data(rightParamList, p);
	lexeme_set_data(firstParam, "head");
	lexeme_set_data(secondParam, "tail");
	return retParamList;
}

static lexeme build_append_list() {
	lexeme retParamList = lexeme_make(PAIR);
	lexeme rightParamList = lexeme_make(PAIR);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	pair p = pair_make(firstParam, rightParamList);
	lexeme_set_data(retParamList, p);
	p = pair_make(secondParam, lexeme_make(NIL));
	lexeme_set_data(rightParamList, p);
	lexeme_set_data(firstParam, "first");
	lexeme_set_data(secondParam, "second");
	return retParamList;
}

static lexeme build_sref_list() {
	lexeme retParamList = lexeme_make(PAIR);
	lexeme rightParamList = lexeme_make(PAIR);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	pair p = pair_make(firstParam, rightParamList);
	lexeme_set_data(retParamList, p);
	p = pair_make(secondParam, lexeme_make(NIL));
	lexeme_set_data(rightParamList, p);
	lexeme_set_data(firstParam, "index");
	lexeme_set_data(secondParam, "str");
	return retParamList;
}

static lexeme build_generic_list(char * name) {
	lexeme head = lexeme_make(PAIR);
	lexeme amp = lexeme_make(AMP);
	lexeme id = lexeme_make(ID);
	pair p = pair_make(amp, lexeme_make(NIL));
	lexeme_set_data(head, p);
	p = pair_make(id, lexeme_make(NIL));
	lexeme_set_data(amp, p);
	lexeme_set_data(id, name);
	return head;
}


static lexeme create_call_return(lexeme evaled) {
	lexeme call = lexeme_make(CALL);
	pair p = pair_make(evaled, lexeme_make(NIL));
	lexeme_set_data(call, p);
	return call;
}
