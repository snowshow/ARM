#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include "can.h"

int CAN_write(int fd, can_t const * packet)
{
	int i;
	int id;
	uint8_t* raw = NULL;
	if ((raw = malloc((packet->length + 4) * sizeof(uint8_t))) == NULL) {
		errno = EINVAL;
		return -1;
	}
	raw[0] = 0xFD;
	id = packet->id;
	raw[1] = id >> 8;
	id &= 0xFF;
	raw[2] = id;
	raw[1] |= packet->length << 4;
	for (i = 0 ; i < packet->length ; i++) {
		raw[i+3] = packet->b[i];
	}
	raw[i+3] = 0xBF;
	if (write(fd, raw, i+4) == -1) {
		return -1;
	}
	fsync(fd);
	return 0;
}
