#include <stdarg.h>
#include <stdint.h>

int CAN_send(int id, int length, ...)
{
	va_list va;
	int i;

	va_start(va, length);
	for (i = 0 ; i < length ; i++) {
		va_arg(va, int);
	}
	va_end(va);

	return 0;
}
