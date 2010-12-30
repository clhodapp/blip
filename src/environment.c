
#include <environment.h>
#include <lexeme.h>
#include <stddef.h>
#include <string.h>
#include <pair.h>

static void find(lexeme env, lexeme f, lexeme *id, lexeme *value);
static lexeme env_id_list(lexeme env); // get first id for an environment
static lexeme env_value_list(lexeme env); // get first value for environment
static void lists_set_id(lexeme env, lexeme id); // set first id
static void lists_set_value(lexeme env, lexeme val); // set first value
static lexeme lists_get_id(lexeme lists);
static lexeme lists_get_value(lexeme lists);
static void env_set_id_list(lexeme env, lexeme idList);
static void env_set_value_list(lexeme env, lexeme valList);
static lexeme env_get_lists(lexeme env);

lexeme env_make() {
	lexeme e = lexeme_make(PAIR); // the extended environment
	lexeme l = lexeme_make(PAIR); // the lists for the environment (id, value)
	pair top = pair_make(l, lexeme_make(NIL));
	pair bottom = pair_make(lexeme_make(NIL), lexeme_make(NIL));

	lexeme_set_data(e, top);
	lexeme_set_data(l, bottom);
	return e;
}

lexeme env_get_parent(lexeme env) {
	pair p = (pair) lexeme_get_data(env);
	return pair_get_left(p);
}

lexeme env_extend(lexeme env, lexeme idList, lexeme valueList) {
	lexeme e = lexeme_make(PAIR); // the extended environment
	lexeme l = lexeme_make(PAIR); // the lists for the environment (id, value)
	pair top = pair_make(env, l);
	pair bottom = pair_make(idList, valueList);

	lexeme_set_data(e, top);
	lexeme_set_data(l, bottom);
	return e;
}

lexeme env_lookup(lexeme env, lexeme id) {
	lexeme temp;
	lexeme value;
	find(env, id, &temp, &value);
	return value;
}

void env_insert(lexeme env, lexeme id, lexeme value) {
	lexeme idInserted = lexeme_make(PAIR);
	lexeme valueInserted = lexeme_make(PAIR);
	pair idData = pair_make(id, env_id_list(env));
	pair valueData = pair_make(value, env_value_list(env));
	lexeme_set_data(idInserted, idData);
	lexeme_set_data(valueInserted, valueData);
	env_set_id_list(env, idInserted);
	env_set_value_list(env, valueInserted);
}

lexeme env_alter(lexeme env, lexeme id, lexeme value) {
	lexeme searchId;
	lexeme searchValue;
	find(env, id, &searchId, &searchValue);

	if (searchValue == lexeme_make(NIL)) return lexeme_make(FALSE);

	lexeme_overwrite(searchValue, value);
	return lexeme_make(TRUE);
}

lexeme env_remove(lexeme env, lexeme id) {
	lexeme currentEnv = env;
	lexeme prevIdPairLexeme = lexeme_make(NIL);
	lexeme prevValuePairLexeme = lexeme_make(NIL);
	lexeme currentIdPairLexeme = env_id_list(env);
	lexeme currentValuePairLexeme = env_value_list(env);
	lexeme nextIdPairLexeme;
	lexeme nextValuePairLexeme;
	pair newIdPair;
	pair newValuePair;
	lexeme prevId;
	lexeme prevValue;
	lexeme currentId;
	lexeme currentValue;

	while (lexeme_get_type(currentEnv) != NIL) {
		currentId = pair_get_left(lexeme_get_data(currentIdPairLexeme));

		// if the first thng needs to be removed, just move the head forward
		if (!strcmp(lexeme_get_data(currentId), lexeme_get_data(id))) {
			env_set_id_list(env, pair_get_right(lexeme_get_data(currentIdPairLexeme)));
			env_set_value_list(env, pair_get_right(lexeme_get_data(currentValuePairLexeme)));
			return lexeme_make(TRUE);
		}

		prevIdPairLexeme = currentIdPairLexeme;
		prevValuePairLexeme = currentValuePairLexeme;
		currentIdPairLexeme = pair_get_right(lexeme_get_data(currentIdPairLexeme)); 
		currentValuePairLexeme = pair_get_right(lexeme_get_data(currentValuePairLexeme));

		while (lexeme_get_type(currentIdPairLexeme) != NIL) {
			currentId = pair_get_left(lexeme_get_data(currentIdPairLexeme));
			currentValue = pair_get_right(lexeme_get_data(currentValuePairLexeme));
			prevId = pair_get_left(lexeme_get_data(prevIdPairLexeme));
			prevValue = pair_get_left(lexeme_get_data(prevValuePairLexeme));
			nextIdPairLexeme = pair_get_right(lexeme_get_data(currentId));
			nextValuePairLexeme = pair_get_right(lexeme_get_data(currentValue));
			if (!strcmp(lexeme_get_data(currentId), lexeme_get_data(id))) {
				newIdPair = pair_make(prevId, nextIdPairLexeme);
				newValuePair = pair_make(prevValue, nextValuePairLexeme);
				return lexeme_make(TRUE);
			}
			currentIdPairLexeme = nextIdPairLexeme;
			currentValuePairLexeme = nextValuePairLexeme;
		}

		currentEnv = env_get_parent(currentEnv);
	}

	return lexeme_make(FALSE);
}

void env_destroy(lexeme env) {
}
			

static void find(lexeme env, lexeme f, lexeme *id, lexeme *value) {
	char * searchString = (char *) lexeme_get_data(f);
	lexeme currentIdPairLexeme;
	lexeme currentValuePairLexeme;
	lexeme currentId;
	lexeme currentValue;
	pair currentIdPair;
	pair currentValuePair;

	while (lexeme_get_type(env) != NIL) {
		currentIdPairLexeme = env_id_list(env);
		currentValuePairLexeme = env_id_list(env);
		while (lexeme_get_type(currentIdPairLexeme) != NIL) {
			currentIdPair = (pair) lexeme_get_data(currentIdPairLexeme);
			currentId = pair_get_left(currentIdPair); 
			currentValuePair = (pair) lexeme_get_data(currentValuePairLexeme);
			if (!strcmp(lexeme_get_data(currentId), searchString)) {
				currentValue = pair_get_left(currentValuePair);
				*id = currentId;
				*value = currentValue;
				return;
			}
			currentIdPairLexeme = pair_get_right(currentIdPair);
			currentValuePairLexeme = pair_get_right(currentValuePair);
		}
		env = env_get_parent(env);
	}

	*id = lexeme_make(NIL);
	*value = lexeme_make(NIL);
}

static lexeme env_id_list(lexeme env) {
	pair p = (pair) lexeme_get_data(env);
	lexeme lists = pair_get_right(p);
	return lists_get_id(lists);
}

static lexeme env_value_list(lexeme env) {
	pair p = (pair) lexeme_get_data(env);
	lexeme lists = pair_get_right(p);
	return lists_get_value(lists);
}

static void env_set_id_list(lexeme env, lexeme id) {
	pair p = lexeme_get_data(env);
	lexeme lists = pair_get_right(p);
	lists_set_id(lists, id);
}

static void env_set_value_list(lexeme env, lexeme value) {
	lexeme lists = env_get_lists(env);
	lists_set_value(lists, value);
}

static void lists_set_id(lexeme lists, lexeme id) {
	lexeme value = lists_get_value(lists);
	pair p = pair_make(id, value);
	lexeme_set_data(lists, p);
}

static void lists_set_value(lexeme lists, lexeme value) {
	lexeme id = lists_get_id(lists);
	pair p = pair_make(id, value);
	lexeme_set_data(lists, p);
}

static lexeme lists_get_id(lexeme lists) {
	pair p = lexeme_get_data(lists);
	return pair_get_left(p);
}

static lexeme lists_get_value(lexeme lists) {
	pair p = lexeme_get_data(lists);
	return pair_get_right(p);
}

static lexeme env_get_lists(lexeme env) {
	pair p = lexeme_get_data(env);
	return pair_get_left(p);
}

