#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "can.h"

int main(int argc, char* argv[])
{
	can_t packet;
	while (1) {
		if (CAN_packet(&packet, 1845, 6, 'a', 'b', 'c', 'd', 'e', 'f') < 0) {
			perror("CAN_packet");
			exit(EXIT_FAILURE);
		}
		CAN_write(STDOUT_FILENO, &packet);
		fflush(stdout);
		usleep(1000000);
	}

	return 0;
}
