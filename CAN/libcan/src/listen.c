/**
 * libcan - CAN library
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

void * bin_listener(void * arg);
void * dec_listener(void * arg);
void * hex_listener(void * arg);
void on_event(can_t packet);

int can_register_callback(can_ctx * ctx, int mask, int filter, void (*cbf)(can_t))
{
	return -1;
}

int can_unregister_callback(can_ctx * ctx, int id)
{
	return -1;
}

int can_listen_on(can_ctx * ctx, int fd, can_f format)
{
	if (ctx->status != 0) {
		pthread_cancel(ctx->pth);
		ctx->status = 0;
		ctx->fd = 0;
	}
	if (fd != 0) {
		ctx->fd = fd;
		pthread_create(ctx->pth, NULL, listener, (void *) format);
	}
	return -1;
}
