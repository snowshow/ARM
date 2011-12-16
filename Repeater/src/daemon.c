#include <unistd.h> // chdir()
#include <stdio.h> // *printf()
#include <stdlib.h> // exit()
#include <string.h> // strlen()

int pipes[2];

void daemonize()
{
	int n;
	char* reply = NULL;
	char buffer[1024];
	
	if (pipe(pipes) < 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	chdir("/"); // Unblock mount point
	if (fork() != 0) {
		if ((n = read(pipes[0], buffer, 1024)) < -1) {
			perror("read");
		} else if (n == 1) {
			exit(EXIT_SUCCESS);
		} else {
			fprintf(stderr, "%s\n", reply);
			exit(EXIT_FAILURE);
		}
	}
	setsid(); // Start new session
	if (fork() != 0) exit(EXIT_SUCCESS); // Fork in new session
	for (int i = 0; i < getdtablesize() ; i++) // Close all file descriptor
		if (i != pipes[1]) // Except pipes to father
			close(i);
}

void reply_to_father(char* reply)
{
	static char zero = '0';
	if (reply == NULL)
		write(pipes[1], &zero, 1);
	else
		write(pipes[1], reply, strlen(reply));
}
