
#include <eval.h>
#include <environment.h>
#include <lexeme.h>
#include <prettyprinter.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <builtins.h>
#include <assert.h>
#include <pair.h>

static lexeme eval_unitlist(lexeme env, lexeme l);
static lexeme eval_lambda(lexeme env, lexeme l);
static lexeme eval_return(lexeme env, lexeme l);
static lexeme eval_call(lexeme env, lexeme l);
static lexeme eval_id(lexeme env, lexeme l);
static lexeme eval_bind(lexeme env, lexeme l);
static lexeme eval_block(lexeme env, lexeme l);
static lexeme eval_args_and_extend(lexeme evalEnv, lexeme funcDefEnv, lexeme params, lexeme args);
lexeme func_obj_make(lexeme env, lexeme params, lexeme body);
static lexeme func_obj_env(lexeme f);
static lexeme func_obj_params(lexeme f);
static lexeme func_obj_body(lexeme f);
static lexeme functionalize_and_listify(lexeme env, lexeme l);
static lexeme eval_and_listify(lexeme env, lexeme l);
static lexeme simplify_call(lexeme env, lexeme call);
static void eval_args(lexeme env, lexeme paramList, lexeme argList, lexeme *retParamList, lexeme *retArgList);
static lexeme delay(lexeme env, lexeme delayed);
static bool checkType(lexeme checked, lexeme_type type);
static lexeme reverse(lexeme list);
//static lexeme resolveCalled(lexeme env, lexeme l);

lexeme (*actions [LEXEME_TYPE_MAX])(lexeme env, lexeme l) = {
	[UNITLIST] = &eval_unitlist,
	[BIND] = &eval_bind,
	[ID] = &eval_bind,
	[CALL] = &eval_call,
	[ID] = &eval_id,
	[LAMBDA] = &eval_lambda,
	[RETURN] = &eval_return,
	[BLOCK] = &eval_block,
};

lexeme eval(lexeme env, lexeme l) {
	lexeme (*action)(lexeme env, lexeme l);
	lexeme_type t;

	t = lexeme_get_type(l);
	action = actions[t];
	
	if (action != NULL) l = action(env, l);
	return l;
}

void eval_init(lexeme env) {
	register_builtins(env);
}

static lexeme eval_return(lexeme env, lexeme l) {
	pair p = lexeme_get_data(l);
	lexeme body = pair_get_left(p);
	if (lexeme_get_type(body) == CALL) {
		return simplify_call(env, body);
	}
	return eval(env, body);
}

static lexeme simplify_call(lexeme env, lexeme call) {
	pair callPair = lexeme_get_data(call);
	pair calledPair;
	lexeme rawCalled = pair_get_left(callPair);
	lexeme callArgs = pair_get_right(callPair);
	lexeme realCalled = eval(env, rawCalled);
	lexeme calledParams;
	lexeme newArgs;
	lexeme newParams;
	lexeme newCall;
	lexeme newCalled;
	lexeme calledRight;

	calledPair = lexeme_get_data(realCalled);
	calledParams = pair_get_left(calledPair);
	calledRight = pair_get_right(calledPair);

	newCall = lexeme_make(CALL);
	newCalled = lexeme_make(lexeme_get_type(realCalled));

	eval_args(env, calledParams, callArgs, &newParams, &newArgs);

	if (lexeme_get_type(realCalled) == BUILTIN) {
		newParams = reverse(newParams);
		newArgs = reverse(newArgs);
	}

	callPair = pair_make(newCalled, newArgs);
	calledPair = pair_make(newParams, calledRight);

	lexeme_set_data(newCall, callPair);
	lexeme_set_data(newCalled, calledPair);

	return newCall;
}

static lexeme eval_unitlist(lexeme env, lexeme l) {
	lexeme left;
	while (l != lexeme_make(NIL)) {
		left = eval(env, pair_get_left(lexeme_get_data(l)));
		if (lexeme_get_type(pair_get_left(lexeme_get_data(l))) == RETURN) return left;
		l = pair_get_right(lexeme_get_data(l));
	}
	return left;
}

static lexeme eval_bind(lexeme env, lexeme l) {
	lexeme id = pair_get_left(lexeme_get_data(l));
	lexeme val = eval(env, pair_get_right(lexeme_get_data(l)));
	assert(val != NULL);
	env_insert(env, id, val);
	return id;
}

static lexeme eval_id(lexeme env, lexeme l) {
	lexeme r = env_lookup(env, l);
	if (r != NULL) {
		return r;
	}
	else {
		fprintf(stderr, "Unbound variable: %s\n",
				(char *) lexeme_get_data(l));
		exit(1);
	}
}

static lexeme eval_call(lexeme env, lexeme l) {
	lexeme called;
	lexeme args;
	lexeme callEnv;
	lexeme builtinArgList;
	lexeme builtinParamList;
	lexeme params;
	lexeme_type type;
	lexeme funcDefEnv; // environment where the function was defined
	lexeme body;
	lexeme tmp;
	while (lexeme_get_type(l) == CALL) {
		called = eval(env, pair_get_left(lexeme_get_data(l)));
		args = pair_get_right(lexeme_get_data(l));
		type = lexeme_get_type(called);
		if (lexeme_get_type(called) == BUILTIN) {
			params = builtin_get_params(called);
			eval_args(env, params, args, &builtinParamList, &builtinArgList);
			lexeme (* action)(lexeme l) = builtin_get_action(called);
			tmp = l;
			l = action(builtinArgList);
			assert(l != NULL);
		}
		else  {
			funcDefEnv = func_obj_env(called);
			params = func_obj_params(called);
			body = func_obj_body(called);
			callEnv = eval_args_and_extend(env, funcDefEnv, params, args);
			if (lexeme_get_type(body) == CALL) {
				l = simplify_call(callEnv, body);
			}
			else {
				l = eval(callEnv, body);
			}
			assert(l != NULL);
		}
		if (lexeme_get_type(l) == CALL) {
			assert(lexeme_get_type(called) == FUNC_OBJ || lexeme_get_type(called) == BUILTIN);
		}
	}
	assert(lexeme_get_type(l) != CALL);
	return l;
}

static lexeme eval_lambda(lexeme env, lexeme l) {
	lexeme params = pair_get_left(lexeme_get_data(l));
	lexeme body = pair_get_right(lexeme_get_data(l));
	lexeme r = func_obj_make(env, params, body);
	return r;
}

static lexeme eval_block(lexeme env, lexeme l) {
	lexeme blockEnv = env_extend(env, lexeme_make(NIL), lexeme_make(NIL));
	return eval(blockEnv, pair_get_left(lexeme_get_data(l)));
}

static lexeme eval_args_and_extend(lexeme evalEnv, lexeme funcDefEnv, lexeme params, lexeme args) {
	lexeme extensionParamList;
	lexeme extensionArgList;
	eval_args(evalEnv, params, args, &extensionParamList, &extensionArgList);
	return env_extend(funcDefEnv, extensionParamList, extensionArgList);
}

static void eval_args(lexeme env, lexeme params, lexeme args, lexeme *retParamList, lexeme *retArgList) {
	lexeme retParamHead = lexeme_make(NIL);
	lexeme retArgHead = lexeme_make(NIL);
	lexeme currentParam;
	lexeme currentArg;
	lexeme newArg;
	lexeme newParam;
	pair p;
	while (lexeme_get_type(args) != NIL) {
		currentParam = pair_get_left(lexeme_get_data(params));
		currentArg = pair_get_left(lexeme_get_data(args));
		if (checkType(currentParam, AMP)) {
			newParam = pair_get_left(lexeme_get_data(currentParam));
			if (lexeme_get_type(currentParam) == DOT) {
				newParam = pair_get_left(lexeme_get_data(currentParam));
				newArg = functionalize_and_listify(env, args);
				assert(lexeme_get_type(newArg) != NIL);
			}
			else {
				newArg = eval_and_listify(env, args);
				assert(lexeme_get_type(newArg) != NIL);
			}
			params = lexeme_make(NIL);
			args = lexeme_make(NIL);
		}
		else {
			if (checkType(currentParam, DOT)) {
				newParam = pair_get_left(lexeme_get_data(currentParam));
				newArg = delay(env, pair_get_left(lexeme_get_data(args)));
				assert(lexeme_get_type(newParam) == ID);
			}
			else {
				newParam = currentParam;
				newArg = eval(env, currentArg);
			}
			params = pair_get_right(lexeme_get_data(params));
			args = pair_get_right(lexeme_get_data(args));
		}
		p = pair_make(newParam, retParamHead);
		retParamHead = lexeme_make(PAIR);
		lexeme_set_data(retParamHead, p);
		p = pair_make(newArg, retArgHead);
		retArgHead = lexeme_make(PAIR);
		lexeme_set_data(retArgHead, p);
	}
	*retParamList = retParamHead;
	*retArgList = retArgHead;
}

lexeme func_obj_make(lexeme env, lexeme params, lexeme body) {
	lexeme r = lexeme_make(FUNC_OBJ);
	lexeme r1 = lexeme_make(PAIR);
	pair p = pair_make(params, r1);
	pair p1 = pair_make(env, body);
	lexeme_set_data(r, p);
	lexeme_set_data(r1, p1);
	return r;
}

static lexeme func_obj_env(lexeme f) {
	pair top = lexeme_get_data(f);
	lexeme l = pair_get_right(top);
	pair bottom = lexeme_get_data(l);
	return pair_get_left(bottom);
}

static lexeme func_obj_params(lexeme f) {
	return pair_get_left(lexeme_get_data(f));
}

static lexeme func_obj_body(lexeme f) {
	pair top = lexeme_get_data(f);
	lexeme l = pair_get_right(top);
	pair bottom = lexeme_get_data(l);
	return pair_get_right(bottom);
}

static lexeme eval_and_listify(lexeme env, lexeme l) {
	lexeme returned;
	lexeme previous;
	lexeme current;
	lexeme prevLeft;
	pair p;

	if (l == lexeme_make(NIL)) return lexeme_make(NIL);

	returned = lexeme_make(PAIR);
	p = pair_make(eval(env, pair_get_left(lexeme_get_data(l))), lexeme_make(NIL));
	lexeme_set_data(returned, p);

	previous = returned;
	l = pair_get_right(lexeme_get_data(l));
	while(!checkType(l, NIL)) {
		current = lexeme_make(PAIR);
		prevLeft = pair_get_left(lexeme_get_data(previous));
		p = pair_make(prevLeft, current);
		lexeme_set_data(previous, p);
		p = pair_make(eval(env, pair_get_left(lexeme_get_data(l))), lexeme_make(NIL));
		lexeme_set_data(current, p);
		l = pair_get_right(lexeme_get_data(l));
		previous = current;
	}

	return returned;
}

static lexeme functionalize_and_listify(lexeme env, lexeme l) {
	lexeme returned;
	lexeme previous;
	lexeme current;
	lexeme prevLeft;
	pair p;

	if (l == lexeme_make(NIL)) return lexeme_make(NIL);

	returned = lexeme_make(PAIR);
	p = pair_make(delay(env, pair_get_left(lexeme_get_data(l))), lexeme_make(NIL));
	lexeme_set_data(returned, p);

	previous = returned;
	l = pair_get_right(lexeme_get_data(l));
	while(!checkType(l, NIL)) {
		current = lexeme_make(PAIR);
		prevLeft = pair_get_left(lexeme_get_data(previous));
		p = pair_make(prevLeft, current);
		lexeme_set_data(previous, p);
		p = pair_make(eval(env, pair_get_left(lexeme_get_data(l))), lexeme_make(NIL));
		lexeme_set_data(current, p);
		l = pair_get_right(lexeme_get_data(l));
		previous = current;
	}

	return returned;
return returned;
}

static lexeme delay(lexeme env, lexeme delayed) {
	return func_obj_make(env, lexeme_make(NIL), delayed);
}

static bool checkType(lexeme checked, lexeme_type type) {
	return lexeme_get_type(checked) == type;
}

static lexeme reverse(lexeme list) {
	lexeme newList = lexeme_make(NIL);
	pair p;
	while (lexeme_get_type(list) != NIL) {
		p = pair_make(pair_get_left(lexeme_get_data(list)), newList);
		newList = lexeme_make(PAIR);
		lexeme_set_data(newList, p);
		list = pair_get_right(lexeme_get_data(list));
	}
	return newList;
}
