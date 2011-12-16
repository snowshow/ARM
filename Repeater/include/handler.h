
int handler_init();
int handler_end(int keep_alive);

int repeater_count();

void lprintf_repeater_list();

int add_repeater(char* service);

int repeater_rm_by_service(char* service, int send_kill_signal);
int repeater_rm_by_pid(int pid, int send_kill_signal);
int repeater_rm_by_id(int id, int send_kill_signal);
void repeater_rm_all();

char* repeater_pidtoservice(int pid);
