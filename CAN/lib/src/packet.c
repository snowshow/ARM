#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "can.h"

int CAN_packet(can_t * packet, int id, int length, ...)
{
    va_list va;
    int i;
	int b;

	if (id > 2048 || id < 0) {
		errno = EINVAL;
		return -1;
	}
	packet->id = id;

	if (length > 8 || length < 0) {
		errno = EINVAL;
		return -1;
	}
	packet->length = length;
	
	va_start(va, length);
	for (i = 0 ; i < length ; i++) {
		b = (uint8_t) va_arg(va, int);
		if (b > 255 || b < 0) {
			errno = EINVAL;
			return -1;
		}
		packet->b[i] = b;
	}
	va_end(va);

	return 0; /* No error */
}
