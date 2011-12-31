#ifndef _LIBCAN_H
#define _LIBCAN_H

#include "can.h"

int itoh(int i, char * h1, char * h2);
int htoi(char c);

int CAN_bin_write(int fd, can_t const * packet);
int CAN_dec_write(int fd, can_t const * packet);
int CAN_hex_write(int fd, can_t const * packet);

#endif
