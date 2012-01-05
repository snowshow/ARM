/*
 * libcan - CAN library
 * 
 * Copyright (C) 2012 7Robot <7robot@list.bde.enseeiht.fr>
 * Wrotten by Élie Bouttier <elie.bouttier@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 
 #include <unistd.h>
 #include <stdarg.h>
 #include <string.h>
 #include <errno.h>
 #include <pthread.h>
 #include <stdio.h>
 
 #include "libcan.h"
 #include "libcan-private.h"
 
 int can_bin_write(int fd, struct can_t const * packet);
 int can_dec_write(int fd, struct can_t const * packet);
 int can_hex_write(int fd, struct can_t const * packet);

/** Initialize a packet from id, length, and bytes values. */
int can_packet_fill(struct can_t * packet, int id, int length, ...)
{
    va_list va;
    int i;
	int b;

	memset(packet, 0, sizeof(struct can_t));
	
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
		can_byte_set(packet, i, b);
	}
	va_end(va);

	return 0;
}


/** Get byte value from byte index. */
uint8_t can_byte_get(struct can_t const * packet, int b)
{
	return *((&packet->b1)+b*sizeof(uint8_t));
}
/** Set byte value from byte index and byte value. */
void can_byte_set(struct can_t * packet, int b, uint8_t c)
{
	*((&packet->b1)+b*sizeof(uint8_t)) = c;
}

/** Write a can packet in specified file descriptor in specified format. */
int can_packet_write(int fd, int format, struct can_t const * packet)
{
	switch (format) {
		case bin:
			return can_bin_write(fd, packet);
		case dec:
			return can_dec_write(fd, packet);
		case hex:
			return can_hex_write(fd, packet);
		default:
			return -1;
	}
}

int can_bin_write(int fd, struct can_t const * packet)
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
		raw[i+3] = can_byte_get(packet, i);
	}
	raw[i+3] = 0xBF;
	if (write(fd, raw, i+4) == -1) {
		return -1;
	}
	fsync(fd);

	return 0;
}

int can_dec_write(int fd, struct can_t const * packet)
{
	char buffer[256];
	sprintf(buffer, "%i", packet->id);
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return -1;
	}
	for (int i = 0 ; i < packet->length ; i++) {
		if (i == 0) {
			sprintf(buffer, "\t%i", can_byte_get(packet, i));
		} else {
			sprintf(buffer, " %i", can_byte_get(packet, i));
		}
		if (write(fd, buffer, strlen(buffer)) == -1) {
			return -1;
		}
	}
	write(fd, "\n", 1);
	fsync(fd);

	return 0;
}

int can_hex_write(int fd, struct can_t const * packet)
{
	char buffer[256];
	sprintf(buffer, "%i", packet->id);
	if (write(fd, buffer, strlen(buffer)) == -1) {
		return -1;
	}
	for (int i = 0 ; i < packet->length ; i++) {
		if (i == 0) {
			sprintf(buffer, "\t%X", can_byte_get(packet, i));
		} else {
			sprintf(buffer, " %02X", can_byte_get(packet, i));
		}
		if (write(fd, buffer, strlen(buffer)) == -1) {
			return -1;
		}
	}
	write(fd, "\n", 1);
	fsync(fd);

	return 0;
}
