#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>
#include <stdint.h>

volatile sig_atomic_t run;

void quit();
void mode_raw(int);

char itoh(int i)
{
	if (i < 9)
		return i + '0';
	else
		return i - 9 + 'A';
}

int main(int argc, char* argv[])
{
	uint8_t c;

	signal(SIGQUIT, quit);
	signal(SIGTERM, quit);
	signal(SIGINT, quit);
	int state = 1;
	int id = 0;
	uint8_t data[8];
	int length = 0;
	run = 1;
	mode_raw(1);
	while (run) {
		if (read(STDIN_FILENO, &c, 1) < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		if (state == 1) {
			if (c == 0xFD)
				state = 2;
		} else if (state == 2) {
			length = c>>4;
			if (length > 8)
				state = 1;
			else {
				id = (c%16)<<8;
				state = 3;
			}
		} else if (state == 3) {
			id += c;
			state = -length;
		} else if (state == 0) {
			if (c == 0xBF) {
				printf("%d\t", id);
				for (int i = length-1 ; i >= 0 ; i--) {
					printf("%c%c ", itoh(data[i]>>4), itoh(data[i]%16));
				}
				printf("\n");
				state = 1;
			} else {
				state = 1;
			}
		} else {
			data[-state-1] = c;
			state++;
		}
	}
	mode_raw(0);

	return 0;
}

void quit(int signal)
{
	run = 0;
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
