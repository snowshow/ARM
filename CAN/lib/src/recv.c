#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "can.h"

static void * listener(void * arg);
static void event(can_t packet);

#define FSIZEINC 8

void (**eventv)(can_t);
int * mask;
int * filter;
int eventc;
int listenfd;
pthread_t thr;
int run;

int CAN_on_event(int m, int f, void (*event)(can_t))
{
	static int fsize = 0; /* Taille du tableau de fonctions eventv */

	if (eventc == fsize) { /* Le tableau est déjà plein */
		if ((eventv = realloc(eventv, (fsize + FSIZEINC) * sizeof(void (*)(can_t)))) == NULL) {
			return -1;
		}
		if ((mask = realloc(mask, (fsize + FSIZEINC) * sizeof(int))) == NULL) {
			return -1;
		}
		if ((filter = realloc(filter, (fsize + FSIZEINC) * sizeof(int))) == NULL) {
			return -1;
		}
		fsize += FSIZEINC;
	}

	eventv[eventc++] = event;
	mask[eventc] = m;
	filter[eventc] = f;

	return 0;
}

int CAN_listen_on(int fd)
{
	static int state = 0;

	if (fd < 0) {
		run = 0;
		state = 0;
		return 0;
	}

	listenfd = fd;
	
	if (state == 0) {
		if ((errno = pthread_create(&thr, NULL, listener, NULL)) != 0) {
			return -1;
		}
		state = 1;
	}
	return 0;
}

static void * listener (void * arg)
{
	can_t packet;
	uint8_t c;
	int state = 1;

	run = 1;
    while (run) {
        if (read(listenfd, &c, 1) < 0) {
			return NULL;
        }
        if (state == 1) {
            if (c == 0xFD)
                state = 2;
        } else if (state == 2) {
            packet.length = c>>4;
            if (packet.length > 8)
                state = 1;
            else {
                packet.id = (c%16)<<8;
                state = 3;
            }
        } else if (state == 3) {
            packet.id += c;
            state = -packet.length;
        } else if (state == 0) {
            if (c == 0xBF) {
				event(packet);
                state = 1;
            } else {
                state = 1;
            }
        } else {
			CAN_set(&packet, packet.length + state, c);
            state++;
        }
    }

	return NULL;
}

static void event(can_t packet)
{
	for (int i = 0 ; i < eventc ; i++) {
		if ((packet.id & mask[i]) == filter[i])
			eventv[i](packet);
	}
}
