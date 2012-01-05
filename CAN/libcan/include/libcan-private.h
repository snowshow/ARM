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

#ifndef _LIBCAN_PRIVATE_H_
#define _LIBCAN_PRIVATE_H_

#define CALLBACK_SIZE_INC 5

#include "libcan.h" /* struct can_t */

#ifdef __cplusplus
extern "C" {
#endif

/** Object reprensenting a callback function. */
struct can_cbf {
    int mask;
    int filter;
    void (*fct)(struct can_t);
};

/** Opaque object representing the library context. */
struct can_ctx {
    int refcount; /* Reference of library count */
	int fd; /* Listen file descriptor */
	enum can_f format; /* Data format on listen file descriptor */
    int inc; /* Incrementation size */
    int asize; /* Array size */
    struct can_cbf ** cbfcts; /* Array of callback functions */
	int status; /* Thread status */
	pthread_t pth; /* Listen thread */
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
