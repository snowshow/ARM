#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "can.h"

can_t* CAN_packet(int id, int length, ...)
{
    va_list va;
    int i;
	int b;

    if (length > 8)
        return NULL;

	can_t* packet = NULL;
	if ((packet = malloc(sizeof(can_t))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	
	if (id > 2048 || id < 0) {
		errno = EINVAL;
		return NULL;
	}
	packet->id = id;
	if (length > 8 || length < 0) {
		errno = EINVAL;
		return NULL;
	}
	packet->length = length;
	packet->data = NULL;

	if((packet->data = malloc(length * sizeof(uint8_t))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	va_start(va, length);
	for (i = 0 ; i < length ; i++) {
		b = (uint8_t) va_arg(va, int);
		if (b > 255 || b < 0) {
			errno = EINVAL;
			return NULL;
		}
		packet->data[i] = b;
	}
	va_end(va);

	return packet;
}

//can_t* truc(void) {
//
//    va_start(va, length);
//    packet->data[0] = 0xFD;
//    packet->data[1] = id >> 8;
//    id %= 256;
//    packet->data[2] = id;
//    packet->data[1] |= length << 4;
//    for (i = 0 ; i < length ; i++) {
//        packet->data[i+3] = va_arg(va, int);
//    }
//    packet->data[length+3] = 0xBF;
//    va_end(va);
//
//    return packet;
//}

