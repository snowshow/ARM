#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

//int canpacket(uint8_t* packet, int id, int length, char* message);
int canpacket(char* packet, int id, int length, ...);

int main (int argc, char* argv[])
{
	char *packet = malloc(sizeof(char));
	memset(packet, 0, 12);
	int length = 6;
	canpacket(packet, 1845, length, 'a', 'b', 0x26, 'd', 'e', 'f');
	while (1) {
		write(STDOUT_FILENO, packet, length+4);
		usleep(1000000);
	}
	return EXIT_SUCCESS;
}

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

/*int main (int argc, char* argv[])
{
	uint8_t *packet = malloc(sizeof(uint8_t));
	memset(packet, 0, 12);
	int length = 8;
	canpacket(packet, 1845, length, "loubout");
	write(STDOUT_FILENO, packet, length+4);
	return EXIT_SUCCESS;
}*/

/*int canpacket(uint8_t* packet, int id, int length, char* message)
{
	va_list va;
	va_start(va, length);
	packet[0] = 0xFB;
	packet[1] = id >> 8;
	id %= 256;
	packet[2] = id;
	packet[1] |= length << 4;
	int i;
	for (i = 0; i < length; i++) {
		packet[i+3] = message[i];
	}
	for (; i < 8; i++) {
		packet[i+3] = 0;
	}
	packet[i+3] = 0xBF;
	return 0;
}*/
