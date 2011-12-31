#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "can.h"

#define DELAY 1000000
#define SEC 1000000

int getargs(int argc, char* argv[], int *delay);

void help(char const * cmd)
{
	printf("Usage: %s [-d delay(ms)] ID B1 B2 ...\n", cmd);
}

int main (int argc, char* argv[])
{
	int delay = DELAY;
	can_t packet;
	unsigned long int randnum;

	int pos = getargs(argc, argv, &delay);

	if (argc - pos == 0) {

		while (1) {

			packet.id = rand() % 2048;
			packet.length = rand() % 9;
			for (int i = 0 ; i < packet.length ; i++) {
				CAN_set(&packet, i, rand() % 256);
			}
			CAN_write(STDOUT_FILENO, &packet, bin);
			usleep(delay);
		}

	} else {

		if (sscanf(argv[pos++], "%d", &(packet.id)) != 1) {
			fprintf(stderr, "Bad value for delay\n");
			help(argv[0]);
			exit(1);
		}
		packet.length = argc - pos;
		int b;
		for (int i = 0 ; i < packet.length ; i++) {
			sscanf(argv[pos++], "%d", &b);
			printf("%i\n", b);
			CAN_set(&packet, i, (uint8_t) b);
		}

		while (1) {
			CAN_write(STDOUT_FILENO, &packet, bin);
			usleep(delay);
		}

	}

	return EXIT_SUCCESS;
}

int getargs(int argc, char* argv[], int *delay)
{
	char options[] = "d:";
	int option;

	while ((option = getopt(argc, argv, options)) != -1) {
		switch (option) {
			case 'd':
				sscanf(optarg, "%d", delay);
				*delay *= 1000;
				break;
			case '?':
				exit(1);
		}
	}

	return optind;
}

