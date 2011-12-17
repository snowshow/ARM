#include <sys/types.h>
#include <sys/socket.h> // listen()
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h> // close()
#include <string.h> // memset()


#include "server.h"
#include "log.h"

int create_tcp_server(const char* hostname, const char* servname)
{
	int listening_socket;

	if ((listening_socket = create_socket_stream(hostname, servname)) < 0)
		return -1;

	if (listen(listening_socket, 8) < 0) {	/* (Socket, Waiting list size) */
		lerror(LOG_WARNING, "listen");
		close(listening_socket);
		return -1;
	}
	
	return listening_socket;
}

int create_socket_stream(const char* hostname, const char* servname)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;

	memset(&hints, 0, sizeof(struct addrinfo)); /* init hints */
	hints.ai_flags = AI_PASSIVE;				/* Not only loopback if hostname is null */
	hints.ai_family = AF_INET;					/* AF_UNSPEC/AF_INET/AF_INET6 */
	hints.ai_socktype = SOCK_STREAM;			/* STREAM(TCP), DGRAM(UDP), SEQPACKET(?) */
	hints.ai_protocol = 0;						/* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(hostname, servname, &hints, &result);
	if (s != 0) {
		lprintf(LOG_NOTICE, "Error: %s", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {

		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		
		if (sfd == -1) {
			lerror(LOG_NOTICE, "socket");
			continue;
		}

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;	/* Success */
		} else {
			lerror(LOG_NOTICE, "bind");
			lprintf(LOG_NOTICE, "Tried address: %s, service: ", inet_ntoa(((const struct sockaddr_in*)(rp->ai_addr))->sin_addr), servname);
		}

		close(sfd);
	}
	freeaddrinfo(result);	/* No longer needed */

	if (rp == NULL) {	/* No address succeeded */
		lprintf(LOG_NOTICE, "Error: Could not bind");
		return -1;
	}

	return sfd;
}
