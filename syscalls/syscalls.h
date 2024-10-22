#ifndef REFMON_SYSCALLS_H
#define REFMON_SYSCALLS_H

extern unsigned long the_syscall_table;

int init_syscalls(void);
void flush_syscalls(void);

typedef struct registered_syscall_codes {
	int sc_code_set_state;
	int sc_code_reconf_add;
	int sc_code_reconf_rm;
	int sc_code_flush;
} registered_syscall_codes;

extern struct registered_syscall_codes syscall_codes;

// the handlers
int do_set_state(char __user *new_state);
int do_reconf_add(char __user *path_in, char __user *password_in);
int do_reconf_rm(char __user *path_in, char __user *password_in);
int do_flush_registry(char __user *password);

#endif //REFMON_SYSCALLS_H
