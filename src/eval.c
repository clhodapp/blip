
#include <eval.h>
#include <environment.h>
#include <lexeme.h>
#include <prettyprinter.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <builtins.h>
#include <assert.h>

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
static lexeme simplifyCall(lexeme env, lexeme call);
static void eval_args(lexeme env, lexeme paramList, lexeme argList, lexeme *retParamList, lexeme *retArgList);
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
	lexeme body = lexeme_get_left(l);
	if (lexeme_get_type(body) == CALL) {
		return simplifyCall(env, body);
	}
	return eval(env, body);
}

static lexeme simplifyCall(lexeme env, lexeme call) {
	lexeme rawCalled = lexeme_get_left(call);
	lexeme callArgs  = lexeme_get_right(call);
	lexeme realCalled = eval(env, rawCalled);
	lexeme callParams;
	lexeme newArgs;
	lexeme newParams;
	lexeme newCall;
	lexeme newCalled;
	if (lexeme_get_type(realCalled) == BUILTIN) {
		return eval(env, call);
	}
	else {
		callParams = lexeme_get_left(realCalled);
		newCall = lexeme_make(CALL);
		newCalled = lexeme_copy(realCalled);
		eval_args(env, callParams, callArgs, &newParams, &newArgs);
		lexeme_set_left(newCall, newCalled);
		lexeme_set_right(newCall, newArgs);
		lexeme_set_left(newCalled, newParams);
		return newCall;
	}
}

static lexeme eval_unitlist(lexeme env, lexeme l) {
	lexeme left;
	while (l != NULL) {
		left = eval(env, lexeme_get_left(l));
		if (lexeme_get_type(lexeme_get_left(l)) == RETURN) return left;
		l = lexeme_get_right(l);
	}
	return left;
}

static lexeme eval_bind(lexeme env, lexeme l) {
	lexeme id = lexeme_get_left(l);
	lexeme val = eval(env, lexeme_get_right(l));
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
		called = eval(env, lexeme_get_left(l));
		args = lexeme_get_right(l);
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
			l = eval(callEnv, body);
			assert(l != NULL);
		}
	}
	assert(lexeme_get_type(l) != CALL);
	return l;
}

static lexeme eval_lambda(lexeme env, lexeme l) {
	lexeme params = lexeme_get_left(l);
	lexeme body = lexeme_get_right(l);
	lexeme r = func_obj_make(env, params, body);
	return r;
}

static lexeme eval_block(lexeme env, lexeme l) {
	lexeme blockEnv = env_extend(env, lexeme_make(NIL), lexeme_make(NIL));
	return eval(blockEnv, lexeme_get_left(l));
}

static lexeme eval_args_and_extend(lexeme evalEnv, lexeme funcDefEnv, lexeme params, lexeme args) {
	lexeme extensionParamList;
	lexeme extensionArgList;
	eval_args(evalEnv, params, args, &extensionParamList, &extensionArgList);
	return env_extend(funcDefEnv, extensionParamList, extensionArgList);
}

static void eval_args(lexeme env, lexeme params, lexeme args, lexeme *retParamList, lexeme *retArgList) {
	lexeme currentParam = params; // Really a list of the remaining parameters. At any given time, we care about the head
	lexeme currentArg = args; // Really a list of the remaining arguments. At any given time, we care about the head
	lexeme pendingParam;
	lexeme pendingArg;
	lexeme retArgs = lexeme_make(NIL);
	lexeme retParams = lexeme_make(NIL);
	lexeme newArg;
	lexeme newParam;
	assert(params != NULL);
	assert(args != NULL);

	while (currentParam != lexeme_make(NIL)) {
		pendingParam = lexeme_get_left(currentParam);
		if (currentArg == lexeme_make(NIL) && lexeme_get_type(pendingParam) != AMP) {
			fprintf(stderr, "Error: too few args\n");
			exit(EXIT_FAILURE);
		}
		if (lexeme_get_type(pendingParam) == DOT) {
			pendingParam = lexeme_get_left(pendingParam);
			pendingArg = func_obj_make(env, lexeme_make(NIL), lexeme_get_left(currentArg));
		}
		else if (lexeme_get_type(pendingParam) == AMP) {
			pendingParam = lexeme_get_left(pendingParam);
			if (lexeme_get_type(pendingParam) == DOT) {
				pendingParam = lexeme_get_left(pendingParam);
				pendingArg = functionalize_and_listify(env, currentArg);
			}
			else {
				pendingArg = eval_and_listify(env, currentArg);
			}

			newParam = lexeme_make(LIST);
			newArg = lexeme_make(LIST);

			lexeme_set_left(newParam, pendingParam);
			lexeme_set_right(newParam, retParams);
			retParams = newParam;

			lexeme_set_left(newArg, pendingArg);
			lexeme_set_right(newArg, retArgs);
			retArgs = newArg;
			currentArg = lexeme_make(NIL); // show we have processes all args
			break;

		}
		else {
			pendingArg = eval(env, lexeme_get_left(currentArg));
		}

		newParam = lexeme_make(LIST);
		newArg = lexeme_make(LIST);

		lexeme_set_left(newParam, pendingParam);
		lexeme_set_right(newParam, retParams);
		retParams = newParam;

		lexeme_set_left(newArg, pendingArg);
		lexeme_set_right(newArg, retArgs);
		retArgs = newArg;

		currentParam = lexeme_get_right(currentParam);
		currentArg = lexeme_get_right(currentArg);
	}
	if (currentArg != lexeme_make(NIL)) {
		fprintf(stderr, "Error: too many args\n");
		exit(EXIT_FAILURE);
	}
	*retParamList = retParams;
	*retArgList = retArgs;
}

lexeme func_obj_make(lexeme env, lexeme params, lexeme body) {
	lexeme r = lexeme_make(FUNC_OBJ);
	lexeme r1 = lexeme_make(FUNC_OBJ);
	lexeme_set_left(r, params);
	lexeme_set_right(r, r1);
	lexeme_set_left(r1, env);
	lexeme_set_right(r1, body);
	return r;
}

static lexeme func_obj_env(lexeme f) {
	return lexeme_get_left(lexeme_get_right(f));
}

static lexeme func_obj_params(lexeme f) {
	return lexeme_get_left(f);
}

static lexeme func_obj_body(lexeme f) {
	return lexeme_get_right(lexeme_get_right((f)));
}

static lexeme eval_and_listify(lexeme env, lexeme l) {
	lexeme returned;
	lexeme previous;
	lexeme current;

	if (l == lexeme_make(NIL)) return l;

	returned = lexeme_make(LIST);
	lexeme_set_left(returned, eval(env, lexeme_get_left(l)));

	previous = returned;
	l = lexeme_get_right(l);
	while(l != lexeme_make(NIL)) {
		current = lexeme_make(LIST);
		lexeme_set_right(previous, current);
		lexeme_set_left(current, eval(env, lexeme_get_left(l)));
		l = lexeme_get_right(l);
		previous = current;
	}

	lexeme_set_right(previous, lexeme_make(NIL));

	return returned;
}

static lexeme functionalize_and_listify(lexeme env, lexeme l) {
//	lexeme returned;
//
//	if (l == NIL_LEXEME) return l;
//
//	returned = lexeme_make(LIST);
//	lexeme_set_left(returned, func_obj_make(env, NIL_LEXEME, lexeme_get_left(l)));
//	lexeme_set_right(returned, functionalize_and_listify(env, lexeme_get_right(l)));
//	return returned;

	lexeme returned;
	lexeme previous;
	lexeme current;

	if (l == lexeme_make(NIL)) return l;

	returned = lexeme_make(LIST);
	lexeme_set_left(returned, func_obj_make(env, lexeme_make(NIL), lexeme_get_left(l)));

	previous = returned;
	l = lexeme_get_right(l);
	while(l != lexeme_make(NIL)) {
		current = lexeme_make(LIST);
		lexeme_set_right(previous, current);
		lexeme_set_left(current, func_obj_make(env, lexeme_make(NIL), lexeme_get_left(l)));
		l = lexeme_get_right(l);
		previous = current;
	}

	lexeme_set_right(previous, lexeme_make(NIL));

	return returned;
}
//static lexeme resolveCalled(lexeme env, lexeme l) {
//	lexeme called = lexeme_get_left(l);
//	lexeme_type type ;
//	if (called == NULL) return NULL;
//	type = lexeme_get_type(called);
//	do {
//		if (type == ID) called = env_lookup(env, called);
//		if (called == NULL) {
//			fprintf(stderr, "ERROR: UNBOUND VARIABLE\n");
//			exit(EXIT_FAILURE);
//		}
//		type = lexeme_get_type(called);
//	} while (type == ID);
//	return called;
//}
