#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "can.h"

void listener(can_t packet)
{
	CAN_hexa_write(1, &packet);
}

int main(int argc, char * argv[])
{
	if (CAN_on_event(0xBFF, 15, listener) < 0) {
		perror("CAN_on_event");
	}
	if (CAN_on_event(0xAFF, 15, listener) < 0) {
		perror("CAN_on_event");
	}
	if (CAN_listen_on(STDIN_FILENO, CT_CAN) < 0) {
		perror("CAN_listen_on");
	}
	while (1)
		sleep(10);
}
