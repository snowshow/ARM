/** Repeater.
 * This program manages repeater' servers.
 */

#include <stdlib.h> // exit()
#include <stdio.h> // perror()
#include <unistd.h> // getopt()
#include <signal.h>
#include <errno.h>
#include <string.h> // strerror()

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"
#include "daemon.h"
#include "sighandler.h"
#include "rephandler.h"
#include "server.h"

//#define DYNAMIC

#ifdef DYNAMIC
#define BUF_SIZE 1024
#else
#define MIN_PORT 3520
#define MAX_PORT 3529
#endif


/** Enregister le pid du daemon dans un fichier. Ceci permet au script de
 * lancement de savoir si le programme est déjà lancé. */
int write_pid(char* pidfilename);
 
int main(int argc, char** argv)
{
	int option;
	char* logfilename = NULL;
	char* pidfilename = NULL;
#ifdef DYNAMIC
	char* handlerserv = NULL;
	int handlersock;
	char* options = "p:l:s:"; // cf man getopt
#else
	char* options = "p:l:"; // cf man getopt
#endif

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
#ifdef DYNAMIC
			case 's': // handler service
				handlerserv = optarg;
				break;
#endif
		}
	}

	// Passer le programme en daemon. cf daemon.c
	daemonize();

	if (logfilename == NULL)
		logfilename = "/var/log/repeater.log"; // Fichiers de log par défaut
	if (loginit(logfilename) < 0) { // Ouverture des log. cf log.c
		reply_to_father("Failed to open log file");
		exit(EXIT_FAILURE);
	}

	lprintf(LOG_INFO, "----------------------------------------------");
	lprintf(LOG_INFO, "Starting repeater …");

	// Initialisation gestionnaire de répéteurs. cf repeater.c:handler_init()
	if (handler_init() < -1) {
		lprintf(LOG_ERR, "at main::handler_init");
		exit(EXIT_FAILURE);
	
	}
	if (pidfilename == NULL)
		pidfilename = "/var/run/repeater.pid"; // Fichier de pid par défaut
	if (write_pid(pidfilename) < 0) { // Définie dans main.c
		lprintf(LOG_NOTICE, "write_pid: Fail to open %s in w+ mode\n", pidfilename);
	}

#ifdef DYNAMIC
	if (handlerserv == NULL)
			handlerserv = "3519";

	if ((handlersock = create_tcp_server(NULL, handlerserv)) < 0) {
		reply_to_father("Can't create main handler server, will exit");
		exit(EXIT_FAILURE);
	}
#else
	char port[16];
	for (int i = MIN_PORT ; i < MAX_PORT ; i++) {
		sprintf(port, "%i", i);
		if (add_repeater(port) < -1)
			lprintf(LOG_ERR, "at main::add_repeater");
	}
	
	lprintf_repeater_list();
#endif
	
	reply_to_father(NULL);

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

#ifdef DYNAMIC
	char buffer[BUF_SIZE];
	int sockets[32];
	int socket;
	int socketc = 0;
	struct sockaddr_in address;
	socklen_t length;

	fd_set fdset;
    while (run) { // Control ?
        FD_ZERO(&fdset);
        FD_SET(handlersock, &fdset);
        for (int i = 0; i < socketc; i++) {
            FD_SET(sockets[i], &fdset);
        }
        if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) <= 0) {
			if (errno != EINTR) {
				lprintf(LOG_WARNING, "select: %s", strerror(errno));
			}
			continue; // Go re-check run
        }
        if (FD_ISSET(handlersock, &fdset)) {
            length = sizeof(struct sockaddr_in);
            socket = accept(handlersock, (struct sockaddr*)(&address), &length);
            if (socket != -1) {
                fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
                sockets[socketc++] = socket;
                lprintf(LOG_INFO, "New connection (total: %i)", socketc);
            } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
                lerror(LOG_ERR, "accept");
                return -1;
            }
        }
		for (int i = 0; i < socketc; i++) {
            if (FD_ISSET(sockets[i], &fdset)) {
                int nbyte;
                if ((nbyte = read(sockets[i], buffer, BUF_SIZE-1)) < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        lerror(LOG_WARNING, "read");
                } else if (nbyte == 0) {
                    close(sockets[i]);
                    sockets[i] = sockets[--socketc];
                    lprintf(LOG_INFO, "Connection closed (remaining: %i)", socketc);
                } else {
					int pos = 0;
					int apos = 0;
					char cmd[20];
					char arg[20];
					
					while (pos < nbyte && buffer[pos] != ' ') {
						cmd[pos] = buffer[pos];
						pos++;
					}
					if (pos == nbyte)
						cmd[pos-1] = '\0';
					else
						cmd[pos] = '\0';
					
					pos++;
					while (pos < nbyte && buffer[pos] != ' ') {
						arg[apos] = buffer[pos];
						pos++; apos++;
					}
					if (pos == nbyte)
						arg[apos-1] = '\0';
					else
						arg[apos] = '\0';
					
					fcntl(socket, F_SETFL, fcntl(sockets[i], F_GETFL) | O_NONBLOCK); // Block mode
					
					char answer[256];
					if (!strcmp(cmd, "START") || !strcmp(cmd, "start")) {
						if (strlen(arg) == 0) {
							printf("No service specified\n");
						} else if (add_repeater(arg) < 0) {
							sprintf(answer, "Failed to start service %s\n", arg);
						} else {
							sprintf(answer, "Service %s started\n", arg);
						}
						send(sockets[i], answer, strlen(answer), 0);
					} else if (!strcmp(cmd, "STOP") || !strcmp(cmd, "stop")) {
						if (strlen(arg) == 0) {
							sprintf(answer, "No service specified\n");
							send(sockets[i], answer, strlen(answer), 0);
						} else if (!strcmp(arg, "ALL") || !strcmp(arg, "all")) {
							repeater_rm_all();
						} else if (repeater_stop_by_service(arg) < 0) {
							sprintf(answer, "Failed to stop service %s\n", arg);
							send(sockets[i], answer, strlen(answer), 0);
						} else {
							sprintf(answer, "Service %s stopped\n", arg);
							send(sockets[i], answer, strlen(answer), 0);
						}
					} else if (!strcmp(cmd, "LIST") || !strcmp(cmd, "list")) {
						int repeaterc = repeater_count();
						for (int id = 0 ; id < repeaterc ; id++) {
							sprintf(answer, "%s\n", get_service_by_id(id));
							send(sockets[i], answer, strlen(answer), 0);
						}
					} else if (!strcmp(cmd, "EXIT") || !strcmp(cmd, "exit") || !strcmp(cmd, "QUIT") || !strcmp(cmd, "quit")) {
						raise(SIGQUIT);
					} else if (!strcmp(cmd, "CLOSE") || !strcmp(cmd, "close")) {
						close(sockets[i]);
						sockets[i] = sockets[--socketc];
						lprintf(LOG_INFO, "Connection closed (remaining: %i)", socketc);
						send(sockets[i], answer, strlen(answer), 0);
					} else {
						cmd[nbyte-1] = '\0'; // Del end-line character
						sprintf(answer, "%s: Unknow command\n", cmd);
						send(sockets[i], answer, strlen(answer), 0);
					}
					
					fcntl(socket, F_SETFL, fcntl(sockets[i], F_GETFL) & ~O_NONBLOCK); // Unblock mode
				}
			}
		}
	}
	close(handlersock);
#else
	sigset_t oldsigset;
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (run)
		sigsuspend(&oldsigset);
#endif

	unlink(pidfilename); // Suppression du fichier de pid
	lprintf(LOG_INFO, "Repeater existed");
	exit(EXIT_SUCCESS);
}

int write_pid(char* filename)
{
	FILE* pidf = NULL;
	if ((pidf = fopen(filename, "w+")) == NULL) {
		lerror(LOG_NOTICE, "fopen");
		return -1;
	}
	fprintf(pidf, "%d", getpid());
	fclose(pidf);
	return 0;
}
