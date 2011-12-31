#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include "libcan.h"
#include "can.h"

int CAN_write(int fd, can_t const * packet, int format)
{
	switch (format) {
		case bin:
			return CAN_bin_write(fd, packet);
		case dec:
			return CAN_dec_write(fd, packet);
		case hex:
			return CAN_hex_write(fd, packet);
		default:
			return -1;
	}
}

int CAN_bin_write(int fd, can_t const * packet)
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
		raw[i+3] = CAN_get(packet, i);
	}
	raw[i+3] = 0xBF;
	if (write(fd, raw, i+4) == -1) {
		return -1;
	}
	fsync(fd);

	return 0;
}

int CAN_dec_write(int fd, can_t const * packet)
{
	char buffer[256];
	sprintf(buffer, "%i", packet->id);
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return -1;
	}
	for (int i = 0 ; i < packet->length ; i++) {
		if (i == 0) {
			sprintf(buffer, "\t%i", CAN_get(packet, i));
		} else {
			sprintf(buffer, " %i", CAN_get(packet, i));
		}
		if (write(fd, buffer, strlen(buffer)) == -1) {
			return -1;
		}
	}
	write(fd, "\n", 1);
	fsync(fd);

	return 0;
}

int CAN_hex_write(int fd, can_t const * packet)
{
	char buffer[256];
	sprintf(buffer, "%i", packet->id);
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return -1;
	}
	for (int i = 0 ; i < packet->length ; i++) {
		char h1, h2;
		itoh(CAN_get(packet, i), &h1, &h2);
		if (i == 0) {
			sprintf(buffer, "\t%c%c", h1, h2);
		} else {
			sprintf(buffer, " %c%c", h1, h2);
		}
		if (write(fd, buffer, strlen(buffer)) == -1) {
			return -1;
		}
	}
	write(fd, "\n", 1);
	fsync(fd);

	return 0;
}
