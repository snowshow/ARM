#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int open_connection (int argc, char** argv)
{
	char* host = "localhost";
	char* service = "3521";
	struct hostent* hostent;
	struct servent* servent;
	int numport;
	int sock;

	if (argc > 1) {
		host = argv[1];
	}
	if (argc > 2) {
		service = argv[2];
	}
	if (argc > 3) {
		fprintf(stdout, "Usage: %s [host] [port]\n", argv[0]);
		return -1;
	}
	struct sockaddr_in * address = NULL;
	if ((address = malloc(sizeof(struct sockaddr_in))) == NULL) {
		perror("malloc");
		return -1;
	}
	memset(address, 0, sizeof(struct sockaddr_in));
	if (inet_aton(host, & (address->sin_addr)) == 0) {
		if ((hostent = gethostbyname(host)) == NULL) {
			fprintf(stderr, "Unknow host (%s)\n", host);
			return -1;
		}
		address->sin_addr.s_addr = ((struct in_addr *) (hostent->h_addr))->s_addr;
	}
	if (sscanf(service, "%d", & numport) == 1) {
		address->sin_port = htons(numport);
	}
	if ((servent = getservbyname(service, "tcp")) == NULL) {
		fprintf(stderr, "Unknow service (%s)\n", service);
		return -1;
	}
	address->sin_port = servent->s_port;
	address->sin_family = AF_INET;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	if (connect(sock, (struct sockaddr *) & address, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		return -1;
	}
	return sock;
}

int main (int argc, char** argv)
{
	struct sockaddr_in address;
	int socket;
	char buffer[BUF_SIZE];
	int n;
	
	if ((socket = open_connection(argc, argv)) < 0) {
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "Socket fd : %i\n", socket);
		//fprintf(stdout, "Connection established to %s port %i\n", address.sin_addr.s_addr, ntohs(address.sin_port));
	}

	//setvbuf(stdout, NULL, _IONBF, 0);
	fd_set fdset;
	while (1) { // ----
		fprintf(stdout, "Wait …\n");
		FD_ZERO(&fdset);
		FD_SET(0, &fdset); // ------
		FD_SET(socket, &fdset);
		if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) <= 0) {
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(socket, &fdset)) {
			fprintf(stdout, "Données à lire sur le socket ?\n");
			if ((n = read(socket, buffer, BUF_SIZE-1)) < 0) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			write(0, buffer, n);
		}
		if (FD_ISSET(0, &fdset)) { // ----
			fprintf(stdout, "Données à lire sur stdin ?\n");
			if ((n = read(0, buffer, BUF_SIZE-1)) < 0) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			write(socket, buffer, n);
		}
	}
	return EXIT_SUCCESS;
}

