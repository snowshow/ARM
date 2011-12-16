/** Repeater' server handler
 */

#include <stdlib.h>
#include <signal.h>
#include <string.h> // strcmp()
#include <unistd.h> // fork()

#include "handler.h"
#include "log.h"
#include "server.h"
#include "repeater.h"

#define MAX_REPEATER 64
#define SERVNAME_LENGTH 256

struct repeater {
	int pid;
	char* service;
};

struct repeater** repeaters;
int repeaterc;

int handler_init()
{
	repeaterc = 0;
	if ((repeaters = malloc(MAX_REPEATER * sizeof(struct repeater*))) == NULL) {
		lerror(LOG_WARNING, "handler_init::malloc");
		return -1;
	}
	return 0;
}

int handler_end(int keep_alive)
{
	if (!keep_alive)
		repeater_rm_all();
	free(repeaters);
	return 0;
}

void lprintf_repeater_list()
{
	for (int i = 0 ; i < repeaterc ; i++) {
		lprintf(LOG_INFO, "Repeater %i : { pid=%i, service=%s }", i, repeaters[i]->pid, repeaters[i]->service);
	}
}

int repeater_count()
{
	return repeaterc;
}

int add_repeater(char* service)
{
	int socket;
	if ((socket = create_tcp_server(NULL, service)) < 0) {
		lprintf(LOG_NOTICE, "create_repeater::create_tcp_server(%s) failed", service);
		return -1;
	}
	int pid = fork();
	switch(pid) {
		case -1: // Error
			lerror(LOG_NOTICE, "fork");
			close(socket);
			return -1;
		case 0: // Son
			lservice(service);
			lprintf(LOG_INFO, "Service %s launched", service);
			if(run_repeater_on(socket) < 0) {
				lprintf(LOG_ERR, "lisen_connection failed");
				exit(EXIT_FAILURE);
			}
			close(socket);
			exit(EXIT_SUCCESS);
		default: // Father
			close(socket);
			struct repeater* rep = malloc(sizeof(struct repeater));
			rep->pid = pid;
			rep->service = malloc((strlen(service)+1) * sizeof(char));
			strcpy(rep->service, service);
			repeaters[repeaterc++] = rep;
	}
	return 0;
}

int repeater_rm_by_service(char* service, int send_kill_signal)
{
	for (int id = 0 ; id < repeaterc ; id++)
		if (strcmp(repeaters[id]->service, service) == 0) {
			return repeater_rm_by_id(id, send_kill_signal);
		}	
	return -1;
}

/** Remove a repeater server from database
 * @param pid Repeater pid
 * @param send_kill_signal Kill repeater or only remove it from database
 * @return 0 on success, -1 on failure
 */
int repeater_rm_by_pid(int pid, int send_kill_signal)
{
	int id;
	
	for (id = 0 ; id < repeaterc ; id++) {
		lprintf(LOG_INFO, "%i ?= %i", repeaters[id]->pid, pid);
		//if (repeaters[id]->pid == pid) {
			//lprintf(LOG_INFO, "repeater_rm_by_pid(): rm repeaters %i (pid=%s)", id, pid);
			//return repeater_rm_by_id(id, send_kill_signal);
			//return 0;
		//}
		//lprintf(LOG_INFO, "repeater_rm_by_pid(): no");
	}
	lprintf(LOG_INFO, "repeater_rm_by_pid(): illegal argument (pid=%s)", pid);
	
	return -1;
}


/** Remove a repeater server from database
 * @param id Repeater id
 * @param send_kill_signal Kill repeater or only remove it from database
 * @return 0 on success, -1 on failure
 */
int repeater_rm_by_id(int id, int send_kill_signal)
{
	lprintf(LOG_INFO, "repeater_rm_by_id(): rm id=%i", id);
	sigset_t allsigmask, backupsigmask;
	
	if (id >= repeaterc || id < 0) {
		lprintf(LOG_INFO, "repeater_rm_by_id(): illegal argument (id=%s)", id);
		return -1;
	}

	
	sigemptyset(&allsigmask);
	sigaddset(&allsigmask, SIGCHLD);
	sigaddset(&allsigmask, SIGCLD);
	sigprocmask(SIG_BLOCK, &allsigmask, &backupsigmask);
	
	if (send_kill_signal)
		kill(repeaters[id]->pid, SIGQUIT);
	
	lprintf(LOG_INFO, "Service %s aborded (pid:%i)", repeaters[id]->service, repeaters[id]->pid);
	free(repeaters[id]->service);
	free(repeaters[id]);
	repeaters[id] = repeaters[--repeaterc];
	raise(SIGUSR1);
	
	sigprocmask(SIG_SETMASK, &backupsigmask, NULL);
	
	return 0;
}

void repeater_rm_all()
{
	while (repeaterc > 0)
		repeater_rm_by_id(repeaterc-1, 1);
	raise(SIGUSR1);
}
