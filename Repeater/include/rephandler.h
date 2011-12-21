#ifndef _REPHANDLER_H
#define _REPHANDLER_H

int handler_init();
void lservice(char* service);
int handler_end(int keep_alive);

int repeater_count();
char* get_service_by_id(int id);

void lprintf_repeater_list();

int add_repeater(char* service);

int repeater_rm_by_service(char* service);
int repeater_rm_by_pid(int pid);
int repeater_rm_by_id(int id);
int repeater_stop_by_service(char* service);
void repeater_rm_all();

#endif
