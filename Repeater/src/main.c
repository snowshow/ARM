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
			handlerserv = "3521";

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

#ifndef DYNAMIC
	sigset_t oldsigset;
	sigprocmask(SIG_BLOCK, &exitsigset, &oldsigset);
	while (run)
		sigsuspend(&oldsigset);
#else
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
                continue;
            }
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
					write(STDOUT_FILENO, buffer, nbyte);
					char cmd[20];
					int pos = 0;
					while (pos < 20 && buffer[pos] != ' ') {
						cmd[pos] = buffer[pos];
						pos++;
					}
					if (strcmp(cmd, "START") == 0) {
						printf("start\n");
					} else if (strcmp(cmd, "STOP") == 0) {
						printf("stop\n");
					} else if (strcmp(cmd, "EXIT") == 0) {
						printf("exit\n");
					} else if (strcmp(cmd, "LIST") == 0) {
						printf("list\n");
					}
                    //sprintf(LOG_INFO, "%i bytes read from socket %i", nbyte, i);
                    //for (int j = 0; j < socketc ; j++) {
                    //    if (i != j) {
                    //        fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) | O_NONBLOCK);
                    //        if (send(sockets[j], buffer, nbyte, 0) < 0) {
                    //            lerror(LOG_WARNING, "send");
                    //        }
                    //        if (fsync(sockets[j]) < 0) {
                    //            lerror(LOG_WARNING, "fsync");
                    //        }
                    //        fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) & ~O_NONBLOCK);
                    //    }
					//}
				}
			}
		}
	}
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
