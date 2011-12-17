#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "rephandler.h"
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
	run = 0;
}

void quit_sig_handler(int signo)
{
	sigset_t allsigmask;
	sigfillset(&allsigmask); /* All signals */
	sigprocmask(SIG_SETMASK, &allsigmask, NULL);
	signal(SIGCHLD, SIG_IGN); /* Ignore sons quit signal */
	repeater_rm_all();
	run = 0;
}

void son_sig_handler(int signo)
{
	sigset_t allsigmask, backupsigmask;
	sigfillset(&allsigmask); /* All signals */
	sigprocmask(SIG_SETMASK, &allsigmask, &backupsigmask);
	int status;
	int pid = wait(&status);
	repeater_rm_by_pid(pid);
	if (status != 0) {
		lprintf(LOG_WARNING, "Process %i quit with status %i", pid, status);
	}
	sigprocmask(SIG_SETMASK, &backupsigmask, NULL);
}
