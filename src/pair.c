
#include <pair.h>
#include <lexeme.h>
#include <stdlib.h>

struct pair_t {
	lexeme left;
	lexeme right;
};

pair pair_make(lexeme left, lexeme right) {
	pair r = malloc(sizeof(struct pair_t));
	r->left = left;
	r->right = right;

	return r;
}

void pair_destroy(pair p) {
	lexeme_destroy(p->left);
	lexeme_destroy(p->right);
	free(p);
}

lexeme pair_get_left(pair p) {
	return p->left;
}

lexeme pair_get_right(pair p) {
	return p->right;
}


