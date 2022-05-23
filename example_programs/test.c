#include <stdio.h>

volatile int x;

int main()
{
	printf("%x %x", &x, x);
	/*
	for(int i = 0; i < 5; ++i)
	{
		printf("hello %d\n", i);
	}
	return 0;
*/
}
