#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "can.h"

#define DELAY 1000000
#define COUNT -1

int getargs(int argc, char * argv[], int * delay, int * count);

void help(char const * cmd)
{
	printf("Usage: %s [-d delay(ms)] [-c count] ID B1 B2 ...\n", cmd);
}

int main (int argc, char* argv[])
{
	int delay = DELAY;
	int count = COUNT;
	can_t packet;

	int pos = getargs(argc, argv, &delay, &count);

	if (argc - pos == 0) {

		struct timeval tv;
		struct timezone tz;
		struct tm * tm;
		gettimeofday(&tv, &tz);
		tm = localtime(&tv.tv_sec);
		srand(((tm->tm_hour*60+tm->tm_min)*60+tm->tm_sec)*1000000+tv.tv_usec);

		while (count) {
			if (count > 0)
				count--;
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

		while (count) {
			if (count > 0)
				count--;
			CAN_write(STDOUT_FILENO, &packet, bin);
			usleep(delay);
		}

	}

	return EXIT_SUCCESS;
}

int getargs(int argc, char * argv[], int * delay, int * count)
{
	char options[] = "d:n:";
	int option;
	int c, d;

	while ((option = getopt(argc, argv, options)) != -1) {
		switch (option) {
			case 'd':
				if (sscanf(optarg, "%d", &d) > 0) {
					*delay = d * 1000;
				}
				break;
			case 'n':
				if (sscanf(optarg, "%d", &c) > 0) {
					*count = c;
				}
				break;
			case '?':
				exit(1);
		}
	}

	return optind;
}

