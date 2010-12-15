
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
	return lexeme_get_data(b);
}

lexeme builtin_get_params(lexeme b) {
	return lexeme_get_left(b);
}

static void register_builtin(lexeme env, char * name, lexeme (*action)(lexeme), lexeme paramList) {
	lexeme id = lexeme_make(ID);
	lexeme registered = lexeme_make(BUILTIN);
	lexeme_set_data(id, name);
	lexeme_set_data(registered, action);
	lexeme_set_left(registered, paramList);
	env_insert(env, id, registered);
}

static lexeme out(lexeme args) {
	args = lexeme_get_left(args);
	while (args != lexeme_make(NIL)) {
		printf("%s", lexeme_to_string(lexeme_get_left(args)));
		args = lexeme_get_right(args);
	}
	return lexeme_make(NIL);
}

static lexeme str_ref(lexeme args) {
	lexeme strLexeme = lexeme_get_left(args);
	long long unsigned int index = ((bigint) lexeme_get_data(lexeme_get_left(lexeme_get_right(args))))->data;
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
	args = lexeme_get_left(args);
	while (args != lexeme_make(NIL)) {
		fprintf(stderr, "%s", lexeme_to_string(lexeme_get_left(args)));
		args = lexeme_get_right(args);
	}
	return lexeme_make(NIL);
}

static lexeme list(lexeme args) {
	return lexeme_get_left(args);
}

static lexeme head(lexeme args) {
	return lexeme_get_left(lexeme_get_left(args));
}

static lexeme tail(lexeme args) {
	return lexeme_get_right(lexeme_get_left(args));
}

static lexeme cons(lexeme args) {
	lexeme returned = lexeme_make(LIST);
	lexeme head = lexeme_get_left(lexeme_get_right(args));
	lexeme tail = lexeme_get_left(args);
	lexeme_set_left(returned, head);
	lexeme_set_right(returned, tail);
	return returned;
}


static lexeme plus(lexeme args) {
	lexeme (*addOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;

	args = lexeme_get_left(args);

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: + requires at least one argument");
	}
	addOp = addOps[lexeme_get_type(lexeme_get_left(args))];
	if (addOp == NULL) {
		fprintf(stderr, "ERROR: Unaddable type");
		exit(EXIT_FAILURE);
	}

	while (lexeme_get_right(args) != lexeme_make(NIL)) {
		current = lexeme_get_left(args);
		right = lexeme_get_right(args);
		next = lexeme_get_left(right);
		new = lexeme_make(ARGLIST);
		lexeme_set_left(new, addOp(current, next));
		lexeme_set_right(new, lexeme_get_right(right));
		args = new;
	}
	return lexeme_get_left(args);
}

static lexeme minus(lexeme args) {
	lexeme (*subOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;

	args = lexeme_get_left(args);

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: + requires at least one argument");
	}
	subOp = subOps[lexeme_get_type(lexeme_get_left(args))];
	if (subOp == NULL) {
		fprintf(stderr, "ERROR: Unsubtractable type");
		exit(EXIT_FAILURE);
	}

	while (lexeme_get_right(args) != lexeme_make(NIL)) {
		current = lexeme_get_left(args);
		right = lexeme_get_right(args);
		next = lexeme_get_left(right);
		new = lexeme_make(ARGLIST);
		lexeme_set_left(new, subOp(current, next));
		lexeme_set_right(new, lexeme_get_right(right));
		args = new;
	}
	return lexeme_get_left(args);
}

static lexeme multiply(lexeme args) {
	lexeme (*mulOp)(lexeme multiplicand1, lexeme multiplicand2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;

	args = lexeme_get_left(args);

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: * requires at least one argument");
	}
	mulOp = mulOps[lexeme_get_type(lexeme_get_left(args))];
	if (mulOp == NULL) {
		fprintf(stderr, "ERROR: Unmultiplyable type");
		exit(EXIT_FAILURE);
	}

	while (lexeme_get_right(args) != lexeme_make(NIL)) {
		current = lexeme_get_left(args);
		right = lexeme_get_right(args);
		next = lexeme_get_left(right);
		new = lexeme_make(ARGLIST);
		lexeme_set_left(new, mulOp(current, next));
		lexeme_set_right(new, lexeme_get_right(right));
		args = new;
	}
	return lexeme_get_left(args);
}

static lexeme divide(lexeme args) {
	lexeme (*divOp)(lexeme addend1, lexeme addend2);
	lexeme current;
	lexeme right;
	lexeme next;
	lexeme new;

	args = lexeme_get_left(args);

	if (args == lexeme_make(NIL)) {
		fprintf(stderr, "Error: + requires at least one argument");
	}
	divOp = divOps[lexeme_get_type(lexeme_get_left(args))];
	if (divOp == NULL) {
		fprintf(stderr, "ERROR: Undividable type");
		exit(EXIT_FAILURE);
	}

	while (lexeme_get_right(args) != lexeme_make(NIL)) {
		current = lexeme_get_left(args);
		right = lexeme_get_right(args);
		next = lexeme_get_left(right);
		new = lexeme_make(ARGLIST);
		lexeme_set_left(new, divOp(current, next));
		lexeme_set_right(new, lexeme_get_right(right));
		args = new;
	}
	return lexeme_get_left(args);
}

lexeme append(lexeme args) {
	lexeme first = lexeme_get_left(lexeme_get_right(args));
	lexeme second = lexeme_get_left(args);
	lexeme retFirst;
	lexeme retSecond = lexeme_make(NIL);
	lexeme currentItem;
	if (second != lexeme_make(NIL)) {
		currentItem = lexeme_make(LIST);
		retSecond = currentItem;
		lexeme_set_left(currentItem, lexeme_get_left(second));
		lexeme_set_right(currentItem, lexeme_make(NIL));
		while (lexeme_get_right(second) != lexeme_make(NIL)) {
			second = lexeme_get_right(second);
			lexeme_set_right(currentItem, lexeme_make(LIST));
			currentItem = lexeme_get_right(currentItem);
			lexeme_set_right(currentItem, lexeme_make(NIL));
			lexeme_set_left(currentItem, lexeme_get_left(second));
		}
	}
	if (first != lexeme_make(NIL)) {
		currentItem = lexeme_make(LIST);
		retFirst = currentItem;
		lexeme_set_left(currentItem, lexeme_get_left(first));
		lexeme_set_right(currentItem, retSecond);
		while (lexeme_get_right(first) != lexeme_make(NIL)) {
			first = lexeme_get_right(first);
			lexeme_set_right(currentItem, lexeme_make(LIST));
			currentItem = lexeme_get_right(currentItem);
			lexeme_set_right(currentItem, retSecond);
			lexeme_set_left(currentItem, lexeme_get_left(first));
		}
	}
	else {
		retFirst = retSecond;
	}
	return retFirst;
}

lexeme nil(lexeme args) {
	return lexeme_make(lexeme_get_left(args) == lexeme_make(NIL) ? TRUE : FALSE);
}

lexeme greater(lexeme args) {
	lexeme x1 = lexeme_get_left(lexeme_get_right(args));
	lexeme x2 = lexeme_get_left(args);
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
	lexeme x1 = lexeme_get_left(lexeme_get_right(args));
	lexeme x2 = lexeme_get_left(args);
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
	*type = lexeme_get_type(lexeme_get_left(args));
	lexeme_set_data(returned, type);
	return returned;
}


static lexeme equal(lexeme args) {
	bool (* equalOp)(lexeme arg1, lexeme arg2);
	args = lexeme_get_left(args);
	lexeme currentVal;
	lexeme nextVal;

	if (args == lexeme_make(NIL) || lexeme_get_right(args) == lexeme_make(NIL)) {
		fprintf(stderr, "ERROR -- EQUAL: Requires at least two arguments");
		exit(EXIT_FAILURE);
	}
	equalOp = equalOps[lexeme_get_type(lexeme_get_left(args))];

	if (equalOp == NULL) {
		equalOp = &equalDefault;
	}

	while (lexeme_get_right(args) != lexeme_make(NIL)) {
		currentVal = lexeme_get_left(args);
		nextVal = lexeme_get_left(lexeme_get_right(args));
		if (!equalOp(currentVal, nextVal)) {
			return lexeme_make(FALSE);
		}
		args = lexeme_get_right(args);
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
	lexeme ifFalse = lexeme_get_left(args);
	lexeme ifTrue = lexeme_get_left(lexeme_get_right(args));
	lexeme conditional = lexeme_get_left(lexeme_get_right(lexeme_get_right(args)));
	return create_call_return(lexeme_get_type(conditional) == TRUE ? ifTrue : ifFalse);
}

static lexeme apply(lexeme args) {
	lexeme callArgList = lexeme_make(NIL);
	lexeme currentArg;
	lexeme newArg;
	lexeme operator = lexeme_get_left(lexeme_get_right(args));
	args = lexeme_get_left(args);


	if (args != lexeme_make(NIL)) {
		callArgList = lexeme_make(ARGLIST);
		lexeme_set_left(callArgList, lexeme_get_left(args));
		currentArg = callArgList;
		args = lexeme_get_right(args);
	}
	while (args != lexeme_make(NIL)) {
		newArg = lexeme_make(ARGLIST);
		lexeme_set_left(newArg, lexeme_get_left(args));
		lexeme_set_right(currentArg, newArg);
		currentArg = newArg;
		args = lexeme_get_right(args);
	}

	if (callArgList != lexeme_make(NIL)) {
		lexeme_set_right(currentArg, lexeme_make(NIL));
	}

	return apply_call(operator, callArgList);

}

static lexeme apply_call(lexeme operator, lexeme callArgList) {
	lexeme callLexeme = lexeme_make(CALL);
	lexeme_set_left(callLexeme, operator);
	lexeme_set_right(callLexeme, callArgList);
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

	newParamList = lexeme_make(LIST);
	dot = lexeme_make(DOT);
	newParam = lexeme_make(ID);
	lexeme_set_right(newParamList, lexeme_make(NIL));
	lexeme_set_left(newParamList, dot);
	lexeme_set_left(dot, newParam);
	lexeme_set_data(newParam, "ifFalse");

	currentParamList = newParamList;

	newParamList = lexeme_make(LIST);
	dot = lexeme_make(DOT);
	newParam = lexeme_make(ID);
	lexeme_set_right(newParamList, currentParamList);
	lexeme_set_left(dot, newParam);
	lexeme_set_left(newParamList, dot);
	lexeme_set_data(newParam, "ifTrue");

	currentParamList = newParamList;

	newParamList = lexeme_make(LIST);
	newParam = lexeme_make(ID);
	lexeme_set_right(newParamList, currentParamList);
	lexeme_set_left(newParamList, newParam);
	lexeme_set_data(newParam, "condition");

	return newParamList;
}

static lexeme build_less_list() {
	lexeme newParamList;
	lexeme currentParamList;
	lexeme newParam;

	newParamList = lexeme_make(LIST);
	newParam = lexeme_make(ID);
	lexeme_set_left(newParamList, newParam);
	lexeme_set_right(newParamList, lexeme_make(NIL));
	lexeme_set_data(newParam, "x1");
	
	currentParamList = newParamList;

	newParamList = lexeme_make(LIST);
	newParam = lexeme_make(ID);
	lexeme_set_right(newParamList, currentParamList);
	lexeme_set_left(newParamList, newParam);
	lexeme_set_data(newParam, "x2");

	return newParamList;
}

static lexeme build_greater_list() {
	lexeme newParamList;
	lexeme currentParamList;
	lexeme newParam;

	newParamList = lexeme_make(LIST);
	newParam = lexeme_make(ID);
	lexeme_set_left(newParamList, newParam);
	lexeme_set_right(newParamList, lexeme_make(NIL));
	lexeme_set_data(newParam, "x1");
	
	currentParamList = newParamList;

	newParamList = lexeme_make(LIST);
	newParam = lexeme_make(ID);
	lexeme_set_right(newParamList, currentParamList);
	lexeme_set_left(newParamList, newParam);
	lexeme_set_data(newParam, "x2");

	return newParamList;
}

static lexeme build_list_list() {
	return build_generic_list("listed");
}

static lexeme build_head_list() {
	lexeme paramlist = lexeme_make(LIST);
	lexeme param = lexeme_make(ID);
	lexeme_set_left(paramlist, param);
	lexeme_set_right(paramlist, lexeme_make(NIL));
	lexeme_set_data(param, "list");
	return paramlist;
}

static lexeme build_tail_list() {
	lexeme paramList = lexeme_make(LIST);
	lexeme param = lexeme_make(ID);
	lexeme_set_left(paramList, param);
	lexeme_set_right(paramList, lexeme_make(NIL));
	lexeme_set_data(param, "list");
	return paramList;
}

static lexeme build_nil_list() {
	lexeme paramList = lexeme_make(LIST);
	lexeme param = lexeme_make(ID);
	lexeme_set_left(paramList, param);
	lexeme_set_right(paramList, lexeme_make(NIL));
	lexeme_set_data(param, "tested");
	return paramList;
}

static lexeme build_apply_list() {
	lexeme retParamList = lexeme_make(LIST);
	lexeme rightParamList = lexeme_make(LIST);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	lexeme_set_left(retParamList, firstParam);
	lexeme_set_right(retParamList, rightParamList);
	lexeme_set_left(rightParamList, secondParam);
	lexeme_set_right(rightParamList, lexeme_make(NIL));
	lexeme_set_data(firstParam, "operator");
	lexeme_set_data(secondParam, "operands");
	return retParamList;
}

static lexeme build_type_list() {
	lexeme paramList = lexeme_make(LIST);
	lexeme param = lexeme_make(ID);
	lexeme_set_left(paramList, param);
	lexeme_set_right(paramList, lexeme_make(NIL));
	lexeme_set_data(param, "arg");
	return paramList;
}

static lexeme build_cons_list() {
	lexeme retParamList = lexeme_make(LIST);
	lexeme rightParamList = lexeme_make(LIST);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	lexeme_set_left(retParamList, firstParam);
	lexeme_set_right(retParamList, rightParamList);
	lexeme_set_left(rightParamList, secondParam);
	lexeme_set_right(rightParamList, lexeme_make(NIL));
	lexeme_set_data(firstParam, "head");
	lexeme_set_data(secondParam, "tail");
	return retParamList;
}

static lexeme build_append_list() {
	lexeme retParamList = lexeme_make(LIST);
	lexeme rightParamList = lexeme_make(LIST);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	lexeme_set_left(retParamList, firstParam);
	lexeme_set_right(retParamList, rightParamList);
	lexeme_set_left(rightParamList, secondParam);
	lexeme_set_right(rightParamList, lexeme_make(NIL));
	lexeme_set_data(firstParam, "first");
	lexeme_set_data(secondParam, "second");
	return retParamList;
}

static lexeme build_sref_list() {
	lexeme retParamList = lexeme_make(LIST);
	lexeme rightParamList = lexeme_make(LIST);
	lexeme firstParam = lexeme_make(ID);
	lexeme secondParam = lexeme_make(ID);
	lexeme_set_left(retParamList, firstParam);
	lexeme_set_right(retParamList, rightParamList);
	lexeme_set_left(rightParamList, secondParam);
	lexeme_set_right(rightParamList, lexeme_make(NIL));
	lexeme_set_data(firstParam, "index");
	lexeme_set_data(secondParam, "str");
	return retParamList;
}

static lexeme build_generic_list(char * name) {
	lexeme head = lexeme_make(LIST);
	lexeme amp = lexeme_make(AMP);
	lexeme id = lexeme_make(ID);
	lexeme_set_left(head, amp);
	lexeme_set_right(head, lexeme_make(NIL));
	lexeme_set_left(amp, id);
	lexeme_set_data(id, name);
	return head;
}


static lexeme create_call_return(lexeme evaled) {
	lexeme call = lexeme_make(CALL);
	lexeme_set_left(call, evaled);
	lexeme_set_right(call, lexeme_make(NIL));
	return call;
}
