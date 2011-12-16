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

/** Enregister le pid du daemon dans un fichier. Ceci permet au script de
 * lancement de savoir si le programme est déjà lancé. */
int write_pid(char* pidfilename);
 
int main(int argc, char** argv)
{
	char* options = "p:l:"; // cf man getopt
	int option;
	char* logfilename = NULL;
	char* pidfilename = NULL;

	// Récupérer les options. Voir man getopt
	opterr = 0; // No auto-error message
	while((option = getopt(argc, argv, options)) != -1) {
		switch(option) {
			case 'p': // pid
				pidfilename = optarg;
				break;
			case 'l': // log
				logfilename = optarg;
				break;
		}
	}

	// Passer le programme en daemon. cf daemon.c
	daemonize();

	if (logfilename == NULL)
		logfilename = "/var/log/repeater.log"; // Fichiers de log par défaut
	if (loginit(logfilename) < 0) { // Ouverture des log. cf log.c
		perror("loginit");
		exit(EXIT_FAILURE);
	}

	lprintf(LOG_INFO, "----------------------------------------------");
	lprintf(LOG_INFO, "Starting server …");
	
	if (pidfilename == NULL)
		pidfilename = "/var/run/repeater.pid"; // Fichier de pid par défaut
	if (write_pid(pidfilename) < 0) { // Définie dans main.c
		lerror(LOG_NOTICE, "write_pid");
	}

	// Initialisation gestionnaire de répéteurs. cf repeater.c:handler_init()
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
	
	reply_to_father(NULL);

	//lprintf_repeater_list();








	
	run = 1;

	// Catch sig quit/interrupt/terminate signal
	install_sig_handler(SIGQUIT, 0, NULL, quit_sig_handler);
	install_sig_handler(SIGINT, 0, NULL, quit_sig_handler);
	install_sig_handler(SIGTERM, 0, NULL, quit_sig_handler);
	// Catch sig child exited
	install_sig_handler(SIGCHLD, SA_RESTART, NULL, son_sig_handler);
	
	
	sigset_t exitsigset;
	sigemptyset(&exitsigset);
	sigaddset(&exitsigset, SIGKILL);
	sigaddset(&exitsigset, SIGTERM);
	sigaddset(&exitsigset, SIGINT);
	
	sigset_t oldsigset;
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (run)
		sigsuspend(&oldsigset);
	
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (repeater_count())
		sigsuspend(&oldsigset);
	
	unlink(pidfilename); // Suppression du fichier de pid
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
