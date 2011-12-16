/** Repeater.
 * This program manages repeater' servers.
 */

#include <stdlib.h> // exit()
#include <stdio.h> // perror()
#include <unistd.h> // getopt()
#include <signal.h>

#include "log.h"
#include "daemon.h"
#include "sighandler.h"
#include "handler.h"

int write_pid(char* pidfilename);
 
int main(int argc, char** argv)
{
	char* options = "p:l:";
	int option;
	char* logfilename = NULL;
	char* pidfilename = NULL;
	
	opterr = 0; /* No auto-error message */
	while((option = getopt(argc, argv, options)) != -1) {
		switch(option) {
			case 'p':
				pidfilename = optarg;
				break;
			case 'l':
				logfilename = optarg;
				break;
		}
	}
	
	daemonize();
	
	if (logfilename == NULL)
		logfilename = "/var/log/repeater.log";
	if (loginit(logfilename) < 0) {
		perror("loginit");
		exit(EXIT_FAILURE);
	}
	
	lprintf(LOG_INFO, "Starting server …");
	
	if (pidfilename == NULL)
		pidfilename = "/var/run/repeater.pid";
	if (write_pid(pidfilename) < 0) {
		lerror(LOG_NOTICE, "write_pid");
	}
	
	if (handler_init() < -1) {
		lprintf(LOG_ERR, "at main::handler_init");
		exit(EXIT_FAILURE);
	}
	
	char port[16];
	for (int i = 3520 ; i < 3522 ; i++) {
		sprintf(port, "%i", i);
		if (add_repeater(port) < -1)
			lprintf(LOG_ERR, "at main::add_repeater");
	}

	//~ lprintf_repeater_list();
	
	reply_to_father(NULL);
	lprintf(LOG_INFO, "Server started");
	
	install_sig_handler(SIGQUIT, 0, NULL, quit_sig_handler);
	//~ install_sig_handler(SIGINT, 0, NULL, quit_sig_handler);
	install_sig_handler(SIGTERM, 0, NULL, quit_sig_handler);
	install_sig_handler(SIGCHLD, SA_RESTART, NULL, son_sig_handler);
	
	sigset_t exitsigset;
	sigemptyset(&exitsigset);
	sigaddset(&exitsigset, SIGKILL);
	sigaddset(&exitsigset, SIGTERM);
	//~ sigaddset(&exitsigset, SIGINT);
	
	run = 1;
	
	sigset_t oldsigset;
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (run)
		sigsuspend(&oldsigset);
	
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (repeater_count())
		sigsuspend(&oldsigset);
	
	unlink(pidfilename);
	lprintf(LOG_INFO, "Server existed");
	exit(EXIT_SUCCESS);
}

int write_pid(char* filename)
{
	FILE* pidf = NULL;
	if ((pidf = fopen(filename, "w+")) == NULL) {
		fprintf(stderr, "fopen: Fail to open %s in w+ mode\n", filename);
		return -1;
	}
	fprintf(pidf, "%d", getpid());
	fclose(pidf);
	return 0;
}
