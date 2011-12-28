#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "can.h"

void truc(can_t packet)
{
	printf("truc\n");
	CAN_write(1, &packet);
}

int main(int argc, char * argv[])
{
	if (CAN_recv(0, 0xBFF, truc) != 0) {
		fprintf(stderr, "Error: CAN_recv\n");
	}
	if (CAN_recv(0, 0xAFF, truc) != 0) {
		fprintf(stderr, "Error: CAN_recv\n");
	}
	sleep(10);
}

void mode_raw(int activer)
{
    static struct termios cooked;
    static int raw_actif = 0;
    
    if (raw_actif == activer)
        return;
    
    if (activer)
    {
        struct termios raw;
        
        tcgetattr(STDIN_FILENO, &cooked);
        
        raw = cooked;
        cfmakeraw(&raw);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
    else
        tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
    
    raw_actif = activer;
}

