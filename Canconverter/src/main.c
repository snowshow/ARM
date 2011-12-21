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

char ntoh(int i)
{
	if (i < 10)
		return i + '0';
	else
		return i - 10 + 'A';
}

int cantostr(char* str, uint8_t* can, int length);

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
	char* str = malloc(256*sizeof(char));
	while (1) {
		if ((n = recv(sock, packet, 12, 0)) == 0) /* EOF */
			break;
		int i;
		for (i = 0; i < n ; i++) {
			fprintf(stdout, "%c%c ", ntoh(packet[i]>>4), ntoh(packet[i]%16));
		}
		fprintf(stdout, "\n");
		int id;
		if ((id = cantostr(str, packet, n)) < 0) {
			fprintf(stdout, "err: %s\n", str);
		} else {
			fprintf(stdout, "%i\t%s", id, str);
		}
		//if (n < 0) {
		//	perror("recv");
		//	exit(EXIT_FAILURE);
		//}
		//write(STDOUT_FILENO, buffer, length);
	}
	return EXIT_SUCCESS;
}

int cantostr(char* str, uint8_t* can, int length)
{
	int id;
	int pos;
	if (length < 4) {
		sprintf(str, "length < 4", strlen("length < 4"));
		return -1;
	} else if (length > 12) {
		sprintf(str, "length > 12", strlen("length > 12"));
		return -1;
	} else if (can[0] != 0xFD) {
		fprintf(stdout, "%i\n", can[0]);
		sprintf(str, "0xFD", strlen("0xFD"));
		return -1;
	} else if (can[length-1] != 0xBF) {
		fprintf(stdout, "can[%i]=%i\n", length-1, can[length-1]);
		sprintf(str, "0xBF", strlen("0xBF"));
		return -1;
	}
	int len = can[1] >> 4;
	id = ((can[1]%16)<<8) + can[2];
	if (len + 4 != length) {
		sprintf(str, "len+4=%i≠lenth=%i", len, length);
		return -1;
	}
	int i;
	for (i = 0 ; i < len ; i++) {
		str[i] = can[3+i];
	}
	str[i] = '\n';
	return id;
}
