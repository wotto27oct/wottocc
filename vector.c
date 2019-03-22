#include "wottocc.h"

Vector *new_vector() {
	Vector *vec = malloc(sizeof(Vector));
	vec->data = malloc(sizeof(void *) * 16);
	vec->capacity = 16;
	vec->len = 0;
	return vec;
}

void vec_push(Vector *vec, void *elem) {
	if (vec->capacity == vec->len) {
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
	}
	vec->data[vec->len++] = elem;
}

void *vec_get(Vector *vec, int num) {
	return vec->data[num];
}

void expect(int line, int expected, int actual) {
	if (expected == actual)
		return;
	fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
	exit(1);
}

void runtest() {
	Vector *vec = new_vector();
	expect(__LINE__, 0, vec->len);

	for (int i = 0; i < 100; i++)
		vec_push(vec, (void *)i);

	expect(__LINE__, 100, vec->len);
	expect(__LINE__, 0, (int)vec->data[0]);
	expect(__LINE__, 50, (int)vec->data[50]);
	expect(__LINE__, 99, (int)vec->data[99]);

	Vector *vec2 = new_vector();
	
	for (int i = 0; i < 100; i++){
		Token tmp;
		tmp.ty = 2*i;
		Token *d = malloc(sizeof(Token));
		*d = tmp;
		vec_push(vec2, (void *)d);
	}
	printf("%d\n", ((Token *)(vec2->data[23]))->ty);
	printf("%d\n", ((Token *)(vec2->data[35]))->ty);
	printf("%d\n", ((Token *)vec_get(vec2, 23))->ty);

	printf("OK\n");
}
