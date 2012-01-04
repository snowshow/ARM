#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "libcan.h"
#include "can.h"

static void * bin_listener(void * arg);
static void * dec_listener (void * arg);
static void * hex_listener (void * arg);
static void event(can_t packet);

#define FSIZEINC 8

void (**eventv)(can_t);
int * mask;
int * filter;
int eventc;
int listenfd;
pthread_t thr;
int type;

int CAN_add_callback(int m, int f, void (*event)(can_t))
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

	return eventc - 1;
}

int CAN_listen_on(int fd, can_f f)
{
	static int state = 0;
	static can_f format = -1;

	if (state != 0) {
		pthread_cancel(thr);
		state = 0;
	}

	if (fd < 0) {
		return 0;
	}

	listenfd = fd;
	
	if (state == 0) {
		format = f;
		switch (format) {
			case bin:
				errno = pthread_create(&thr, NULL, bin_listener, NULL);
				break;
			case dec:
				errno = pthread_create(&thr, NULL, dec_listener, NULL);
				break;
			case hex:
				errno = pthread_create(&thr, NULL, hex_listener, NULL);
				break;
			default:
				errno = EINVAL;
		}
		if (errno != 0) {
			return -1;
		}
		state = 1;
	}

	return 0;
}

static void * bin_listener (void * arg)
{
	can_t packet;
	uint8_t c;
	int state = 1;

    while (1) {
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

static void * dec_listener (void * arg)
{
	can_t packet;
	char c;
	int state = 0;
	int n = 0;

	while (1) {
		if (read(listenfd, &c, 1) < 0) {
			return NULL;
		}
		if (c == '\n') {
			if (state == 0) {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					packet.length = 0;
					event(packet);
				}
			} else if (state < 9) {
				if (n >= 0 && n < 256) {
					CAN_set(&packet, state-1, n);
					packet.length = state;
					event(packet);
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
					CAN_set(&packet, state-1, n);
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

static void * hex_listener (void * arg)
{
	can_t packet;
	char c;
	int state = 0;
	int n = 0;
	int t = 0;

	while (1) {
		if (read(listenfd, &c, 1) < 0) {
			return NULL;
		}
		if (c == '\n') {
			if (state == 0) {
				if (n >= 0 && n < 2048) {
					packet.id = n;
					packet.length = 0;
					event(packet);
				}
			} else if (state < 9) {
				if (n >= 0 && n < 256) {
					CAN_set(&packet, state-1, n);
					packet.length = state;
					event(packet);
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
					CAN_set(&packet, state-1, n);
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
		} else {
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
