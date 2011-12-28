#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "can.h"

#define DELAY 1000000
#define SEC 1000000

int getargs(int argc, char* argv[], int *delay);

int main (int argc, char* argv[])
{
	int delay = DELAY;
	can_t packet;
	CAN_packet(&packet, 15, 3, 'a', 'b', 'c');

	int pos = getargs(argc, argv, &delay);
	printf("%ii\n", pos);

	while (1) {
		CAN_write(STDOUT_FILENO, &packet);
		usleep(delay);
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
				sscanf(optarg, "%d", delay);
				break;
			case '?':
				exit(1);
		}
	}

	return option;
}

