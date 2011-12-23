#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "can.h"

#define DELAY 1000000
#define SEC 1000000


int main (int argc, char* argv[])
{
	char *packet = malloc(sizeof(char));
	memset(packet, 0, 12);
	int length = 6;
	canpacket(packet, 1845, length, 'a', 'b', 0x26, 'd', 'e', 'f');
	int delay = DELAY;

	while (1) {
		write(STDOUT_FILENO, packet, length+4);
		usleep(DELAY);
	}
	return EXIT_SUCCESS;
}

int getargs(int argc, char* argv[], int *delay)
{
	char options[] = "d:";
	int option;

	while ((option = getopt(argc, argv, options)) < 0) {
		switch (option) {
			case 'd':
				scanf("%d", optarg);
				break;
			case '?':
				exit(1);
		}
	}
}

