/** Repeater' server handler
 */

#include <stdlib.h>
#include <signal.h>
#include <string.h> // strcmp()
#include <unistd.h> // fork()

#include "rephandler.h"
#include "log.h"
#include "server.h"
#include "repeater.h"
#include "sighandler.h"

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
		lprintf(LOG_INFO, "Repeater %i : { service=%s, pid=%i  }", i, repeaters[i]->service, repeaters[i]->pid);
	}
}

int repeater_count()
{
	return repeaterc;
}

char* get_service_by_id(int id)
{
	if (id < repeaterc)
		return repeaters[id]->service;
	else
		return NULL;
}

int add_repeater(char* service)
{
	int socket;
	if ((socket = create_tcp_server(NULL, service)) < 0) {
		lprintf(LOG_NOTICE, "Failed to start service %s", service);
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
			lprintf(LOG_INFO, "Service %s started", service);
			if(run_repeater_on(socket) < 0) {
				lprintf(LOG_ERR, "lisen_connection failed");
				exit(EXIT_FAILURE);
			}
			lprintf(LOG_INFO, "Service %s stopped", service);
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

/** Remove a repeater server from database
 * @param service Repeater servie
 * @return 0 on success, -1 on failure
 */
int repeater_rm_by_service(char* service)
{
	for (int id = 0 ; id < repeaterc ; id++)
		if (strcmp(repeaters[id]->service, service) == 0) {
			return repeater_rm_by_id(id);
		}	
	return -1;
}

/** Remove a repeater server from database
 * @param pid Repeater pid
 * @return 0 on success, -1 on failure
 */
int repeater_rm_by_pid(int pid)
{
	int id;
	
	for (id = 0 ; id < repeaterc ; id++) {
		if (repeaters[id]->pid == pid) {
			return repeater_rm_by_id(id);
		}
	}
	lprintf(LOG_INFO, "repeater_rm_by_pid(): illegal argument (pid=%d)", pid);
	
	return -1;
}


/** Remove a repeater server from database
 * @param id Repeater id
 * @return 0 on success, -1 on failure
 */
int repeater_rm_by_id(int id)
{
	sigset_t allsigmask, backupsigmask;
	
	if (id >= repeaterc || id < 0) {
		lprintf(LOG_INFO, "repeater_rm_by_id(): illegal argument (id=%s)", id);
		return -1;
	}

	sigemptyset(&allsigmask);
	sigaddset(&allsigmask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &allsigmask, &backupsigmask);
	
	//lprintf(LOG_INFO, "Service %s shutdown", repeaters[id]->service, repeaters[id]->pid);
	free(repeaters[id]->service);
	free(repeaters[id]);
	repeaters[id] = repeaters[--repeaterc];
	
	sigprocmask(SIG_SETMASK, &backupsigmask, NULL);
	
	return 0;
}

int repeater_stop_by_service(char* service)
{
	for (int i = 0; i < repeaterc; i++) {
		if (!strcmp(repeaters[i]->service, service)) {
			kill(repeaters[i]->pid, SIGQUIT);
			return 0;
		}
	}
	return -1;
}

void repeater_rm_all()
{
	signal(SIGCHLD, SIG_IGN);
	for (int i = 0; i < repeaterc; i++) {
		lprintf(LOG_INFO, "Send quit signal to service %s (pid %i) …", repeaters[i]->service, repeaters[i]->pid);
		kill(repeaters[i]->pid, SIGQUIT);
	}
	repeaterc = 0;
	usleep(100000); // Là j'ai pas d'idée de solution miracle :/
	install_sig_handler(SIGCHLD, SA_RESTART, NULL, son_sig_handler);
}
