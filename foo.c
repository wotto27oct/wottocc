#include "stdio.h"

int foo()
{
	printf("foo!\n");
	return 0;
}


int footwo(int x)
{
	printf("%d\n", x);
	return 0;
}

int foothree(int x, int y)
{
	printf("%d\n", x + 2 * y);
	return 0;
}

int foosix(int a, int b, int c, int d, int e, int f)
{
	printf("%d\n", a + b + c + d + e + f);
	return 0;
}
