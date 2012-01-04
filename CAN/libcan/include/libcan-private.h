/**
 * libcan - CAN library
 */

#ifndef _LIBCAN_PRIVATE_H_
#define _LIBCAN_PRIVATE_H_

#include "libcan.h" /* struct can_t */

#ifdef __cplusplus
extern "C" {
#endif

/** Object reprensenting a callback function. */
struct can_cbf {
    int mask;
    int filter;
    void (*cbf)(struct can_t);
};

/** Opaque object representing the library context. */
struct can_ctx {
    int refcount; /* Reference of library count */
	int fd; /* Listen file descriptor */
    int inc; /* Incrementation size */
    int fvsize; /* Array size */
    struct can_cbf * cbfcts; /* Array of callback functions */
	int status; /* Thread status */
	pthread_t pth; /* Listen thread */
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
