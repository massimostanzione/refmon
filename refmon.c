#include "refmon.h"
#include "syscalls/syscalls.h"
#include "probing/probing.h"
#include "io/procio.h"
#include "model/instance.h"
#include "model/state.h"
#include "misc/misc.h"

unsigned long the_syscall_table = 0x0;
module_param(the_syscall_table, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

int init_password(void)
{
	char *enc;
	pr_debug("%s: initializing password... ", REFMON_MODNAME);
	enc = hashgen_str(REFMON_DEFAULT_PASSWORD);
	if (enc == NULL) {
		pr_err("%s: error while hashing password", REFMON_MODNAME);
		return -ENOTRECOVERABLE;
	}
	the_instance.password = enc;
	pr_cont("done.");
	return 0;
}

int init_instance(void)
{
	pr_debug("%s: initializing instance fields... ", REFMON_MODNAME);
	the_instance.state = OFF;
	INIT_LIST_HEAD(&the_instance.registry);
	if (!check_list_init(&the_instance.registry)) {
		pr_err("%s: error while trying to set up refmon instance",
		       REFMON_MODNAME);
		return -ENOTRECOVERABLE;
	}
	pr_cont("done.");
	return 0;
}

void cleanup_module()
{
	pr_info("%s: removing...", REFMON_MODNAME);

	flush_procfiles();
	flush_syscalls();
	flush_probes();

	pr_cont("done, bye");
}

int init_module(void)
{
	int ret;
	pr_info("%s: initializing.", REFMON_MODNAME);

	ret = init_password();
	if (ret != 0) {
		pr_alert("%s: cannot initialize password, aborting (ret=%d)",
			 REFMON_MODNAME, ret);
		goto abort;
	}

	ret = init_instance();
	if (ret != 0) {
		pr_alert(
			"%s: cannot initialize instance values, aborting (ret=%d)",
			REFMON_MODNAME, ret);
		goto abort;
	}

	ret = init_procfiles();
	if (ret != 0) {
		pr_alert("%s: cannot initialize /proc files, aborting (ret=%d)",
			 REFMON_MODNAME, ret);
		goto abort;
	}

	ret = init_syscalls();
	if (ret != 0) {
		pr_alert("%s: cannot initialize syscalls, aborting (ret=%d)",
			 REFMON_MODNAME, ret);
		goto abort;
	}

	ret = init_probes();
	if (ret != 0) {
		pr_alert("%s: cannot initialize probes, aborting (ret=%d)",
			 REFMON_MODNAME, ret);
		goto abort;
	}

	pr_info("%s: ready, enjoy!", REFMON_MODNAME);
	return 0;

abort:
	cleanup_module();
	pr_info("%s: init_module aborted", REFMON_MODNAME);
	return ret;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Massimo Stanzione");
MODULE_DESCRIPTION("Reference monitor, for enhanced file access protection.");

