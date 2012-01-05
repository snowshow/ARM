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

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "libcan.h"
#include "libcan-private.h"

typedef struct can_ctx can_ctx;
typedef struct can_t can_t;

void dispatch(can_ctx * ctx, can_t packet);
void * listener(void * arg);
void * bin_listener(void * arg);
void * dec_listener(void * arg);
void * hex_listener(void * arg);
void * txt_listener(void * arg);
int htoi(char c);

int can_register_callback(can_ctx * ctx, int mask, int filter, void (*cbf)(can_t))
{
	int i;
	
	for (i = 0 ; i < ctx->asize ; i++) {
		if (ctx->cbfcts[i] == NULL)
			break;
	}
	if (i == ctx->asize) { /* End of array reached without finding empty cell */
		ctx->cbfcts = realloc(ctx->cbfcts, (ctx->asize + ctx->inc) * sizeof(struct can_cbf));
		if (ctx->cbfcts == NULL)
			return -1;
		ctx->asize += ctx->inc;
		for (int j = i ; j < ctx->asize ; j++) {
			ctx->cbfcts[j] = NULL;
		}
	}
	if ((ctx->cbfcts[i] = calloc(1, sizeof(struct can_cbf))) == NULL)
		return -1;
	ctx->cbfcts[i]->mask = mask;
	ctx->cbfcts[i]->filter = filter;
	ctx->cbfcts[i]->fct = cbf;
	
	return 0;
}

int can_unregister_callback(can_ctx * ctx, int id)
{
	if (id >= ctx->asize || ctx->cbfcts[id] == NULL) {
		errno = EINVAL;
		return -1;
	}
	free(ctx->cbfcts[id]);
	ctx->cbfcts[id] = NULL;
	return 0;
}

int can_listen_on(can_ctx * ctx, int fd, enum can_f format)
{
	if (ctx->status != 0) {
		pthread_cancel(ctx->pth);
		ctx->status = 0;
		ctx->fd = 0;
	}
	if (fd != 0) {
		ctx->fd = fd;
		ctx->format = format;
		pthread_create(&(ctx->pth), NULL, listener, (void *) ctx);
	}
	return -1;
}

void dispatch(can_ctx * ctx, can_t packet)
{
	int i;
	struct can_cbf * cbf;
	
	for (i = 0 ; i < ctx->asize ; i++) {
		cbf = ctx->cbfcts[i];
		if (cbf != NULL) {
			if ((packet.id & cbf->mask) == cbf->filter) {
				cbf->fct(packet);
			} 
		}
	}
}

void * listener(void * arg)
{
	((can_ctx *) arg)->status = 1;
	switch(((can_ctx *) arg)->format) {
		case bin:
			bin_listener(NULL);
			break;
		case dec:
			dec_listener(NULL);
			break;
		case hex:
			hex_listener(NULL);
			break;
		case txt:
			txt_listener(NULL);
			break;
	}
	((can_ctx *) arg)->status = 0;
	return NULL;
}


void * bin_listener (void * arg)
{
	can_ctx * ctx = (can_ctx *) arg;
	can_t packet;
	uint8_t c;
	int state = 1;
	
	while (1) {
		if (read(ctx->fd, &c, 1) < 0) {
			return NULL;
		}
		if (state == 1) {
			if (c == 0xFD) {
				state = 2;
			}
		} else if (state == 2) {
			packet.length = c>>4;
			if (packet.length > 8) {
				state = 1;
			} else {
				packet.id = (c%16)<<8;
				state = 3;
			}
		} else if (state == 3) {
			packet.id += c;
			state = -packet.length;
		} else if (state == 0) {
			if (c == 0xBF) {
				dispatch(ctx, packet);
				state = 1;
			} else {
				state = 1;
			}
		} else {
			can_byte_set(&packet, packet.length + state, c);
			state++;
		}
	}
	
	return NULL;
}

void * dec_listener (void * arg)
{
	can_ctx * ctx = (can_ctx *) arg;
	can_t packet;
	char c;
	int state = 0;
	int n = 0;

	while (1) {
		if (read(ctx->fd, &c, 1) < 0) {
			return NULL;
		}
		if (c == '\n') {
			if (state == 0) {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					packet.length = 0;
					dispatch(ctx, packet);
				}
			} else if (state < 9) {
				if (n >= 0 && n < 256) {
					can_byte_set(&packet, state-1, n);
					packet.length = state;
					dispatch(ctx, packet);
				}
			}
			state = 0;
			n = 0;
		} else if (state == 0) {
			if (c == '\t') {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					n = 0;
					state = 1;
				} else {
					state = 9;
				}
			} else if (c >= '0' && c <= '9') {
				n *= 10;
				n += c - '0';
			} else {
				state = 9;
			}
		} else if (state > 0 && state < 9) {
			if (c == ' ' && state != 8) {
				if (n >= 0 && n < 256) {
					can_byte_set(&packet, state-1, n);
					state++;
					n = 0;
				} else {
					state = 9;
				}
			} else if (c >= '0' && c <= '9') {
				n *= 10;
				n += c - '0';
			} else {
				state = 9;
			}
		} else {
		}
	}
	return NULL;
}

void * hex_listener (void * arg)
{
	can_ctx * ctx = (can_ctx *) arg;
	can_t packet;
	char c;
	int state = 0;
	int n = 0;
	int t = 0;

	while (1) {
		if (read(ctx->fd, &c, 1) < 0) {
			return NULL;
		}
		if (c == '\n') {
			if (state == 0) {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					packet.length = 0;
					dispatch(ctx, packet);
				}
			} else if (state < 9) {
				if (n >= 0 && n < 256) {
					can_byte_set(&packet, state-1, n);
					packet.length = state;
					dispatch(ctx, packet);
				}
			}
			state = 0;
			n = 0;
		} else if (state == 0) {
			if (c == '\t') {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					n = 0;
					state = 1;
				} else {
					state = 9;
				}
			} else if (c >= '0' && c <= '9') {
				n *= 10;
				n += c - '0';
			} else {
				state = 9;
			}
		} else if (state > 0 && state < 9) {
			if (c == ' ' && state != 8) {
				if (n >= 0 && n < 256) {
					can_byte_set(&packet, state-1, n);
					state++;
					n = 0;
				} else {
					state = 9;
				}
			} else if ((t = htoi(c)) < 0) {
				state = 9;
			} else {
				n *= 16;
				n += t;
			}
		}
	}
	return NULL;
}

int htoi(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
	} else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
	} else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
	} else
        return -1;
}
