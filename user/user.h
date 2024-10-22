#ifndef REFMON_USER_H
#define REFMON_USER_H

#include <stdio.h>

#define MATCH_COMMAND(cmd) \
	if (strcmp(long_options[option_index].name, cmd) == 0)

#define RETURN_OK 0

#define SC_NO 4
#define SC_ORDER_SET_STATE 0
#define SC_ORDER_RECONF_ADD 1
#define SC_ORDER_RECONF_RM 2
#define SC_ORDER_FLUSH 3

#define PROC_USER_SYSCALL_FILE "/proc/refmon-syscallcodes"
#define PROC_USER_STATE_FILE "/proc/refmon-state"
#define PROC_USER_LIST_FILE "/proc/refmon-list"

int get_syscall_index(int index);
int read_procfile_from_user(char *filepath);
char *ask_for_password();

#endif //REFMON_USER_H
