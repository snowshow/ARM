#include <stdint.h>

#include "can.h"

void CAN_set(can_t * packet, int b, uint8_t c)
{
	*((&packet->b1)+b*sizeof(uint8_t)) = c;
}

uint8_t CAN_get(can_t const * packet, int b)
{
	return *((&packet->b1)+b*sizeof(uint8_t));
}
