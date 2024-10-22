#include "syscalls.h"
#include "../lib/scth/scth.h"
#include "../io/procio.h"
#include "../probing/probing.h"
#include "../misc/fsutils.h"
#include "../misc/misc.h"
#include "../misc/kernelvers_limits.h"
#include "../refmon.h"

struct registered_syscall_codes syscall_codes;

unsigned long the_ni_syscall;
unsigned long new_sys_call_array[] = { 0x0, 0x0, 0x0, 0x0 };
#define HACKED_ENTRIES (int)(sizeof(new_sys_call_array) / sizeof(unsigned long))
int restore[HACKED_ENTRIES] = { [0 ...(HACKED_ENTRIES - 1)] - 1 };

#if LINUX_VERSION_CODE >= REFMON_KERNELVER_SYSCALLDEF
__SYSCALL_DEFINEx(1, _set_state, char *, new_state)
{
#else
asmlinkage long sys_set_state(char *new_state)
{
#endif
	return do_set_state(new_state);
}

#if LINUX_VERSION_CODE >= REFMON_KERNELVER_SYSCALLDEF
__SYSCALL_DEFINEx(2, _reconf_add, char *, path, char *, password)
{
#else
asmlinkage long sys_reconf_add(char *path, char *password)
{
#endif
	return do_reconf_add(path, password);
}

#if LINUX_VERSION_CODE >= REFMON_KERNELVER_SYSCALLDEF
__SYSCALL_DEFINEx(2, _reconf_rm, char *, path, char *, password)
{
#else
asmlinkage long sys_reconf_rm(char *path, char *password)
{
#endif
	return do_reconf_rm(path, password);
}

#if LINUX_VERSION_CODE >= REFMON_KERNELVER_SYSCALLDEF
__SYSCALL_DEFINEx(1, _flush, char *, password)
{
#else
asmlinkage long sys_flush(char *password)
{
#endif
	return do_flush_registry(password);
}

long sc_addr_set_state = (unsigned long)__x64_sys_set_state;
long sc_addr_reconf_add = (unsigned long)__x64_sys_reconf_add;
long sc_addr_reconf_rm = (unsigned long)__x64_sys_reconf_rm;
long sc_addr_flush = (unsigned long)__x64_sys_flush;

int init_syscalls(void)
{
	int i, ret = REFMON_RETVAL_DEFAULT;
	if (the_syscall_table == 0x0) {
		pr_alert(
			"%s: cannot manage sys_call_table address set to 0x0, aborting.",
			REFMON_MODNAME);
		return -1;
	}

	pr_debug("%s: vtpmo received sys_call_table address %px\n",
		 REFMON_MODNAME, (void *)the_syscall_table);
	pr_debug("%s: initializing - hacked entries %d\n", REFMON_MODNAME,
		 HACKED_ENTRIES);

	new_sys_call_array[0] = (unsigned long)sc_addr_set_state;
	new_sys_call_array[1] = (unsigned long)sc_addr_reconf_add;
	new_sys_call_array[2] = (unsigned long)sc_addr_reconf_rm;
	new_sys_call_array[3] = (unsigned long)sc_addr_flush;

	ret = get_entries(restore, HACKED_ENTRIES,
			  (unsigned long *)the_syscall_table, &the_ni_syscall);

	if (ret != HACKED_ENTRIES) {
		pr_alert("%s: could not hack %d entries (just %d)\n",
			 REFMON_MODNAME, HACKED_ENTRIES, ret);
		return -1;
	}

	unprotect_memory();

	for (i = 0; i < HACKED_ENTRIES; i++) {
		((unsigned long *)the_syscall_table)[restore[i]] =
			(unsigned long)new_sys_call_array[i];
	}

	protect_memory();
	pr_info("%s: all new system-calls correctly installed on sys-call table\n",
		REFMON_MODNAME);

	syscall_codes.sc_code_set_state = restore[0];
	syscall_codes.sc_code_reconf_add = restore[1];
	syscall_codes.sc_code_reconf_rm = restore[2];
	syscall_codes.sc_code_flush = restore[3];

	return 0;
}

void flush_syscalls(void)
{
	int i;
	
	pr_info("%s: flushing syscalls... ", REFMON_MODNAME);

	unprotect_memory();
	for (i = 0; i < HACKED_ENTRIES; i++) {
		((unsigned long *)the_syscall_table)[restore[i]] =
			the_ni_syscall;
	}
	protect_memory();

	pr_cont("done.");
}
