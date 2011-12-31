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
#define BUF_SIZE 1024

int run_repeater_on(int listening_socket)
{
	struct sockaddr_in address;
	socklen_t length = sizeof(struct sockaddr_in);
	int sockets[MAX_CONNECTION];
	int socketc = 0;
	int socket;
	
	char listen_buffer[BUF_SIZE];
	int listen;
	char target_buffer[MAX_CONNECTION][BUF_SIZE];
	int target[MAX_CONNECTION];
	int written;
	int pending;
	
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

	fd_set listenfdset;
	fd_set targetfdset;

	while (run) { // accès conccurent ?
		
		FD_ZERO(&listenfdset);
		FD_SET(listening_socket, &listenfdset);
		for (int i = 0; i < socketc; i++) {
			FD_SET(sockets[i], &listenfdset);
		}

		if (select(FD_SETSIZE, &listenfdset, NULL, NULL, NULL) < 0) {
			if (errno != EINTR) {
				lerror(LOG_WARNING, "select (listenfdset)");
			} else {
				continue;
			}
		}

		if (FD_ISSET(listening_socket, &listenfdset)) {
			// Client connection
			socket = accept(listening_socket, (struct sockaddr*)(&address), &length);
			if (socket != -1) {
				fcntl(socket, F_SETFL, fcntl(listening_socket, F_GETFL) | O_NONBLOCK);
				target[socketc] = 0;
				sockets[socketc++] = socket;
				lprintf(LOG_INFO, "%i connections [+1]", socketc);
			} else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				lerror(LOG_ERR, "accept");
				return -1;
			}
		}
		for (int i = 0; i < socketc; i++) {
			if (FD_ISSET(sockets[i], &listenfdset)) {
				// There are data to read
				if ((listen = read(sockets[i], listen_buffer, BUF_SIZE)) <= 0) {
					if (listen < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != ECONNRESET)
						lerror(LOG_WARNING, "read");
					close(sockets[i]);
					sockets[i] = sockets[--socketc];
					//lprintf(LOG_INFO, "%i connections [-1]", socketc);
				} else {
					for (int j = 0 ; j < socketc ; j++) {
						if (j != i) {
							memcpy(target_buffer[j], listen_buffer, listen);
							target[j] = listen;
						}
					}
					//lprintf(LOG_INFO, "%i bytes read, go writting", listen);
					while (1) {
						pending = 0;
						FD_ZERO(&targetfdset);
						for (int i = 0 ; i < socketc ; i++) {
							if (target[i] > 0) {
								pending++;
								FD_SET(sockets[i], &targetfdset);
							}
						}
						//lprintf(LOG_INFO, "%i target remaining", pending);
						if (!pending)
							break;
						if (select(FD_SETSIZE, NULL, &targetfdset, NULL, NULL) < 0) {
							if (errno != EINTR) {
								lerror(LOG_WARNING, "select (targetfdset)");
							} else {
								continue;
							}
						}
						for (int i = 0 ; i < socketc ; i++) {
							if (FD_ISSET(sockets[i], &targetfdset)) {
								if ((written = write(sockets[i], target_buffer[i], target[i])) < 0) {
									lerror(LOG_WARNING, "write");
								} else if (written != 0) {
									memmove(target_buffer[i], &(target_buffer[i][written]), target[i] - written);
									target[i] -= written;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
