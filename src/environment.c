
#include <environment.h>
#include <lexeme.h>
#include <stddef.h>
#include <string.h>

static void advance(lexeme *l);
static void find(lexeme env, lexeme f, lexeme *id, lexeme *value);
static lexeme env_id_list(lexeme env); // get first id for an environment
static lexeme env_val_list(lexeme env); // get first value for environment
static lexeme env_next(lexeme env); // get next environment
static void frame_set_id(lexeme env, lexeme id); // set first id
static void frame_set_val(lexeme env, lexeme val); // set first value
static lexeme frame_id(lexeme frame);
static lexeme frame_val(lexeme frame);
static void env_set_next(lexeme env, lexeme next); // sets next environment
static void env_set_id_list(lexeme env, lexeme idList);
static void env_set_val_list(lexeme env, lexeme valList);
static lexeme env_frame(lexeme env);
static void env_set_frame(lexeme env, lexeme frame);

lexeme env_make() {
	lexeme e = lexeme_make(LIST);
	lexeme f = lexeme_make(FRAME);

	env_set_frame(e, f);
	env_set_next(e, lexeme_make(NIL));
	frame_set_id(f, lexeme_make(NIL));
	frame_set_val(f, lexeme_make(NIL));
	return e;
}

lexeme env_parent(lexeme env) {
	return env_next(env);
}

lexeme env_extend(lexeme env, lexeme idList, lexeme valueList) {
	lexeme e = lexeme_make(LIST);
	lexeme f = lexeme_make(FRAME);


	env_set_frame(e, f);
	env_set_next(e, env);
	frame_set_id(f, idList);
	frame_set_val(f, valueList);
	return e;
}

lexeme env_lookup(lexeme env, lexeme id) {
	lexeme temp;
	lexeme value;
	find(env, id, &temp, &value);
	return value;
}

void env_insert(lexeme env, lexeme id, lexeme value) {
	lexeme idInserted = lexeme_make(LIST);
	lexeme valueInserted = lexeme_make(LIST);
	lexeme_set_left(idInserted, id);
	lexeme_set_left(valueInserted, value);
	lexeme_set_right(idInserted, env_id_list(env));
	lexeme_set_right(valueInserted, env_val_list(env));
	env_set_id_list(env, idInserted);
	env_set_val_list(env, valueInserted);
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
	if (env == lexeme_make(NIL)) return lexeme_make(FALSE);
	char * searchString = (char *) lexeme_get_data(id);
	lexeme currentId = env_id_list(env);
	lexeme currentValue = env_val_list(env);
	lexeme prevId = lexeme_make(NIL);
	lexeme prevValue = lexeme_make(NIL);
	if (currentId == lexeme_make(NIL)) return env_remove(env_next(env), id);
	while (strcmp(searchString, lexeme_get_data(lexeme_get_left(currentId)))) {
		prevId = currentId;
		prevValue = currentValue;
		advance(&currentId);
		advance(&currentValue);
		if (currentId == lexeme_make(NIL)) return env_remove(env_next(env), id);
	}
	if (prevId == lexeme_make(NIL)) {
		env_set_id_list(env, lexeme_get_right(currentId));
		env_set_val_list(env, lexeme_get_right(currentValue));
	}
	else {
		lexeme_set_right(prevId, lexeme_get_right(currentId));
		lexeme_set_right(prevValue, lexeme_get_right(currentValue));
	}
	return lexeme_make(TRUE);
}

void env_destroy(lexeme env) {
}
			

static void advance(lexeme *l) {
	*l = lexeme_get_right(*l);
}

static void find(lexeme env, lexeme f, lexeme *id, lexeme *value) {
	if (env == lexeme_make(NIL)) {
		*id = lexeme_make(NIL);
		*value = lexeme_make(NIL);
		return;
	}
	char * searchString = (char *) lexeme_get_data(f);
	lexeme currentId = env_id_list(env);
	lexeme currentValue  = env_val_list(env);

	if (currentId == lexeme_make(NIL)) {
		find(env_next(env), f, &currentId, &currentValue);
		*id = currentId;
		*value = currentValue;
		return;
	}

	while (strcmp(lexeme_get_data(lexeme_get_left(currentId)), searchString)) {
		advance(&currentId);
		advance(&currentValue);
		if (currentId == lexeme_make(NIL)) {
			find(env_next(env), f, &currentId, &currentValue);
			*id = currentId;
			*value = currentValue;
			return;
		}
	}
	*id = lexeme_get_left(currentId);
	*value = lexeme_get_left(currentValue);
	return;
}

static lexeme env_id_list(lexeme env) {
	lexeme frame = env_frame(env);
	return frame_id(frame);
}

static lexeme env_val_list(lexeme env) {
	lexeme frame = env_frame(env);
	return frame_val(frame);
}

static lexeme env_next(lexeme env) {
	return lexeme_get_right(env);
}

static void env_set_id_list(lexeme env, lexeme id) {
	lexeme frame = env_frame(env);
	frame_set_id(frame, id);
}

static void env_set_val_list(lexeme env, lexeme val) {
	lexeme frame = env_frame(env);
	frame_set_val(frame, val);
}

static void env_set_next(lexeme env, lexeme next) {
	lexeme_set_right(env, next);
}

static void frame_set_id(lexeme frame, lexeme id) {
	lexeme_set_left(frame, id);
}

static void frame_set_val(lexeme frame, lexeme val) {
	lexeme_set_right(frame, val);
}

static lexeme frame_id(lexeme frame) {
	return lexeme_get_left(frame);
}

static lexeme frame_val(lexeme frame) {
	return lexeme_get_right(frame);
}

static lexeme env_frame(lexeme env) {
	return lexeme_get_left(env);
}

static void env_set_frame(lexeme env, lexeme frame) {
	lexeme_set_left(env, frame);
}

