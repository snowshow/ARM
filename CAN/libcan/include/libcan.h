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

#ifndef _LIBCAN_H_
#define _LIBCAN_H_

#include <stdint.h> /* uint8_t */

#ifdef __cplusplus
extern "C" {
#endif

/***********************
 * CAN library context *
 ***********************/

/** Opaque object representing the library context. */
struct can_ctx;
/** Create can context library. */
int can_new(struct can_ctx ** ctx);
/** Take a reference of the can library context. */
struct can_ctx * can_ref(struct can_ctx * ctx);
/** Drop a reference of the can library context. If the refcount reaches zero,
 * the ressources of the context will be released. */
int can_unref(struct can_ctx * ctx);


/**************************************************
 * CAN packet representative structure and format *
 **************************************************/

/** Transparent object reprensenting a CAN packet. */
struct can_t {
    int id; /* ID (from 0 to 2047) */
    int length; /* Packet length (from 0 to 8) */
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
    uint8_t b5;
    uint8_t b6;
    uint8_t b7;
    uint8_t b8;
};

/** Enumeration of can bus data format. */
enum can_f {
	bin, /* Binaire : ce qui circule sur le bus série */
	dec, /* Décimal : ex: 1235	12 23 42 (notez une tabulation puis des espaces) */
	hex, /* Hexadécimal : ex: 1235	0C 17 2A */
	txt /* Texte : les valeurs sont converties en chaînes de textes significatives */
};


/*************************
 * CAN managment packets *
 *************************/
 
/** Initialize a packet from id, length, and bytes values. */
int can_packet_fill(struct can_t * packet, int id, int length, ...);
/** Get byte value from byte index. */
uint8_t can_byte_get(struct can_t const * packet, int b);
/** Set byte value from byte index and byte value. */
void can_byte_set(struct can_t * packet, int b, uint8_t c);

/** Write a can packet in specified file descriptor in specified format. */
int can_packet_write(int fd, int format, struct can_t const * packet);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
