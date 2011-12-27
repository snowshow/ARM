#include <stdlib.h>
#include <pthread.h>

#include "can.h"

void * listener(void * arg);

void (*on_event)(can_t*);

int CAN_recv(int fd, void (*event)(can_t*))
{
	static pthread_t thr;
	int error;

	on_event = event;

	if ((error = pthread_create(&thr, NULL, listener, NULL)) != 0) {
		return error;
	}

	return 0;
}

void * listener(void * arg)
{
	return NULL;
}
