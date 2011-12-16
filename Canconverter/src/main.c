#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024


int getargs (int argc, char** argv, struct sockaddr_in * address, char * protocol)
{
	char* options = "a:p:h";
	int option;
	char* host = "localhost";
	char* service = "3521";
	struct hostent* hostent;
	struct servent* servent;
	int numport;
	
	while ((option = getopt(argc, argv, options)) != -1) {
		switch (option) {
			case 'a':
				host = optarg;
				break;
			case 'p':
				service = optarg;
				break;
			case 'h':
			default:
				fprintf(stdout, "Usage : %s [-a address] [-p port]\n", argv[0]);
				return -1;
		}
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
		return 0;
	}
	if ((servent = getservbyname(service, protocol)) == NULL) {
		fprintf(stderr, "Unknow service (%s)\n", service);
		return -1;
	}
	address->sin_port = servent->s_port;
	return 0;
}

int main (int argc, char** argv)
{
	struct sockaddr_in address;
	int sock;
	uint8_t packet[12];
	char buffer[BUF_SIZE];
	int n;
	int length;
	
	if (getargs(argc, argv, & address, "tcp") < 0)
		exit(EXIT_FAILURE);
	address.sin_family = AF_INET;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if (connect(sock, (struct sockaddr *) & address, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
	while (1) {
		if ((n = recv(sock, packet, 12, 0)) == 0) /* EOF */
			break;
		//~ int i;
		//~ for (i = 0; i < n ; i++) {
			//~ fprintf(stdout, "[%c](%i) ", packet[i], packet[i]);
			//~ fprintf(stdout, "%c%c ", ctoh(packet[i]>>4), ctoh(packet[i]%16));
		//~ }
		//~ fprintf(stdout, "\n");
		if (n < 0) {
			perror("recv");
			exit(EXIT_FAILURE);
		}
		write(STDOUT_FILENO, buffer, length);
	}
	return EXIT_SUCCESS;
}

