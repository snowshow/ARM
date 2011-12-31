#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX 2000

int main(int argc, char * argv[])
{
	int c[MAX] = { 0 };
	int t = 0;
	int a;
	unsigned long int r;
	while (t < 100000000) {
		t++;
		a = rand() % MAX;
		c[a]++;
	}
	int i;
	for (i = 0 ; i < MAX ; i++) {
		printf("%i ", c[i]);
	}
	printf("\n");
}
