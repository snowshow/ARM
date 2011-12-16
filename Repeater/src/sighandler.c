#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "handler.h"
#include "sighandler.h"
#include "log.h"

int install_sig_handler(int signo, int flags,
	struct sigaction* oldsate, void(*sig_handler)(int))
{
	struct sigaction action;
	
	action.sa_handler = sig_handler; /* SIG_IGN: ignore, SIG_DFL: default action */
	sigemptyset(&(action.sa_mask)); /* no signal blocked during execution of signal handler */
	action.sa_flags = flags; /* SA_RESTART: restart interruptible function after,
								SA_RESETHAND: restore default after handler execution */
	
	if (sigaction(signo, &action, oldsate) != 0) { /* NULL : don't need to save old state */
		return -1;
	} else {
		return 0;
	}
}

void listenquit_sig_handler(int signo)
{
	lprintf(LOG_INFO, "lisenquit_sig_handler: Quit signal received (%i: %s)", signo, sys_siglist[signo]);
	run = 0;
}

void quit_sig_handler(int signo)
{
	sigset_t allsigmask;
	sigfillset(&allsigmask); /* All signals */
	sigprocmask(SIG_SETMASK, &allsigmask, NULL);
	lprintf(LOG_INFO, "quit_sig_handler: Quit signal received (%i: %s)", signo, sys_siglist[signo]);
	signal(SIGCHLD, SIG_IGN); /* Ignore sons quit signal */
	repeater_rm_all();
	lprintf(LOG_INFO, "quit_sig_handler: All repeater killed");
	run = 0;
}

void son_sig_handler(int signo)
{
	sigset_t allsigmask, backupsigmask;
	sigfillset(&allsigmask); /* All signals */
	sigprocmask(SIG_SETMASK, &allsigmask, &backupsigmask);
	int status;
	int pid = wait(&status);
	lprintf(LOG_INFO, "sin_sig_handler: Son quit signal received (%i: %s, pid: %i)", signo, sys_siglist[signo], pid);
	repeater_rm_by_pid(pid, 0);
	lprintf(LOG_NOTICE, "removed");
	if (status != 0) {
		lprintf(LOG_NOTICE, "Process %i quit with status %i", pid, status);
	}
	sigprocmask(SIG_SETMASK, &backupsigmask, NULL);
}
