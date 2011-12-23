#include <stdarg.h>

int canpacket(char* packet, int id, int length, ...)
{
    va_list va;
    int i;

    if (length > 8)
        return -1;

    va_start(va, length);
    packet[0] = 0xFD;
    packet[1] = id >> 8;
    id %= 256;
    packet[2] = id;
    packet[1] |= length << 4;
    for (i = 0 ; i < length ; i++) {
        packet[i+3] = va_arg(va, int);
    }
    packet[length+3] = 0xBF;
    va_end(va);

    return 0;
}
