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
