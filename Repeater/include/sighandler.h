#ifndef _SIGHANDLER_H
#define _SIGHANDLER_H

volatile sig_atomic_t run;

int install_sig_handler(int signo, int flags,
	struct sigaction* oldsate, void(*sig_handler)(int));
	
void quit_sig_handler(int signo);
void son_sig_handler(int signo);
void listenquit_sig_handler(int signo);

#endif
