/**
 * libcan - CAN library
 */

#include <stdlib.h> /* NULL */

#include "libcan.h"
#include "libcan-private.h"

#define SIZEINC 5

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
	c->inc = SIZEINC;
	c->fvsize = 0;
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
