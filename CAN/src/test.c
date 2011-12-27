#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "can.h"

int main(int argc, char* argv[])
{
	can_t* packet;
	while (1) {
		packet = NULL;
		packet = CAN_packet(1845, 6, 'a', 'b', 'c', 'd', 'e', 'f');
		if (packet == NULL) {
			perror("CAN_packet");
			exit(1);
		}
		CAN_send_packet(1, packet);
		fflush(stdout);
		sleep(1);
	}

	return 0;
}
