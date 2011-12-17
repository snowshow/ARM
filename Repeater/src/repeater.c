#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "log.h"
#include "sighandler.h"

#define MAX_CONNECTION 64
#define BUF_SIZE 128

int run_repeater_on(int listening_socket)
{
	struct sockaddr_in address;
	socklen_t length;
	int sockets[MAX_CONNECTION];
	int socketc = 0;
	int socket;
	char buffer[BUF_SIZE];
	
	/* accept is an exemple of interruptible function, that should not be restarted on SIGQUIT, SIGINT, … */
	install_sig_handler(SIGQUIT, 0, NULL, listenquit_sig_handler);
	install_sig_handler(SIGINT, 0, NULL, listenquit_sig_handler);
	install_sig_handler(SIGTERM, 0, NULL, listenquit_sig_handler);
	
	sigset_t exitsigset;
	sigemptyset(&exitsigset);
	sigaddset(&exitsigset, SIGKILL);
	sigaddset(&exitsigset, SIGTERM);
	sigaddset(&exitsigset, SIGINT);
	
	fcntl(listening_socket, F_SETFL, fcntl(listening_socket, F_GETFL) | O_NONBLOCK);
	
	run = 1;

	fd_set fdset;
	while (run) { // Control ?
		FD_ZERO(&fdset);
		FD_SET(listening_socket, &fdset);
		for (int i = 0; i < socketc; i++) {
			FD_SET(sockets[i], &fdset);
		}
		if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) <= 0) {
			if (errno != EINTR) {
				lprintf(LOG_WARNING, "select: %s", strerror(errno));
				continue;
			}
		}
		if (FD_ISSET(listening_socket, &fdset)) {
			length = sizeof(struct sockaddr_in);
			socket = accept(listening_socket, (struct sockaddr*)(&address), &length);
			if (socket != -1) {
				fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) | O_NONBLOCK);
				sockets[socketc++] = socket;
				lprintf(LOG_INFO, "Socket %i connected", socketc);
			} else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				lerror(LOG_ERR, "accept");
				return -1;
			}
		}
		for (int i = 0; i < socketc; i++) {
			if (FD_ISSET(sockets[i], &fdset)) {
				//~ lprintf(LOG_INFO, "Read data from socket %i", i);
				int nbyte;
				if ((nbyte = read(sockets[i], buffer, BUF_SIZE-1)) < 0) {
					if (errno != EAGAIN && errno != EWOULDBLOCK)
						lerror(LOG_WARNING, "read");
				} else if (nbyte == 0) {
					lprintf(LOG_INFO, "Socket %i closed", i);
					close(sockets[i]);
					sockets[i] = sockets[--socketc];
				} else {
					//~ sprintf(LOG_INFO, "%i bytes read from socket %i", nbyte, i);
					for (int j = 0; j < socketc ; j++) {
						if (i != j) {
							fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) | O_NONBLOCK);
							if (send(sockets[j], buffer, nbyte, 0) < 0) {
								lerror(LOG_WARNING, "send");
							}
							if (fsync(sockets[j]) < 0) {
								lerror(LOG_WARNING, "fsync");
							}
							fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) & ~O_NONBLOCK);
						}
					}
				}
			}
		}
	}
	return 0;
}
