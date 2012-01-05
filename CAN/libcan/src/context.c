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

#include <stdlib.h> /* NULL */

#include "libcan.h"
#include "libcan-private.h"

typedef struct can_ctx can_ctx;

/** Create can context library.
 * This function return -1 if malloc fail and 0 on success. */
int can_new(struct can_ctx ** ctx)
{
	can_ctx * c = NULL;
	if ((c = malloc(sizeof(can_ctx))) == NULL) {
		return -1;
	}
	c->refcount = 1;
	c->fd = 0;
	c->inc = CALLBACK_SIZE_INC;
	c->asize = 0;
	c->cbfcts = NULL;
	*ctx = c;
	return 0;
}

/** Take a reference of the can library context. */
struct can_ctx * can_ref(struct can_ctx * ctx)
{
	if (ctx == NULL)
		return NULL;
	ctx->refcount++;
	return ctx;
}

/** Drop a reference of the can library context. If the refcount reaches zero,
 * the ressources of the context will be released. */
int can_unref(struct can_ctx * ctx)
{
	if (ctx == NULL) {
		return -1;
	}
	ctx->refcount--;
	if (ctx->refcount == 0) {
		free(ctx);
	}
	return 0;
}
