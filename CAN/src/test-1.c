#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "can.h"

void listener(can_t packet)
{
	write(STDOUT_FILENO, "{", 1);
	CAN_write(1, &packet);
	write(STDOUT_FILENO, "}\n", 2);
}

int main(int argc, char * argv[])
{
	if (CAN_on_event(0xBFF, 15, listener) < 0) {
		perror("CAN_on_event");
	}
	if (CAN_on_event(0xAFF, 15, listener) < 0) {
		perror("CAN_on_event");
	}
	if (CAN_listen_on(STDIN_FILENO) < 0) {
		perror("CAN_listen_on");
	}
	sleep(10);
}
