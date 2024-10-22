#include <linux/version.h>
#include "files.h"
#include "procio.h"
#include "../syscalls/syscalls.h"
#include "../model/instance.h"
#include "../model/registry.h"
#include "../misc/kernelvers_limits.h"
#include "../refmon.h"
#include "../misc/misc.h"

#if LINUX_VERSION_CODE < REFMON_KERNELVER_PROCMGT
struct file_operations procops_syscalls = {
	.read = read_proc_syscalls,
};
struct file_operations procops_state = {
	.read = read_proc_state,
};
struct file_operations procops_list = {
	.read = read_proc_list,
};
#else
struct proc_ops procops_syscalls = {
	.proc_read = read_proc_syscalls,
};
struct proc_ops procops_state = {
	.proc_read = read_proc_state,
};
struct proc_ops procops_list = {
	.proc_read = read_proc_list,
};
#endif

ssize_t _read_proc(struct file *file, char __user *buf, size_t count,
		   loff_t *pos, char *name)
{
	char out[PAGE_SIZE];
	int ret = 0, decrement = PAGE_SIZE * sizeof(char);

	if (MATCH_STRING(name, REFMON_FILE_REGISTRY)) {
		SPIN_INSTANCE_LOCK
		struct registry_entry *nodeP;
		list_for_each_entry(nodeP, &the_instance.registry,
				    registry_noderef) {
			ret = snprintf(out + ret, decrement,
				       "- %lu %s -> %lu %s\n",
				       nodeP->registered.ino_no,
				       nodeP->registered.path,
				       nodeP->resolved.ino_no,
				       nodeP->resolved.path);

			if (ret < 0) {
				SPIN_INSTANCE_UNLOCK
				pr_err("%s: error while trying to snprintf to proc file (file='%s')",
				       REFMON_MODNAME, name);
				ret = -EIO;
				goto tail;
			}

			if (ret >= decrement) {
				pr_err("%s: buffer overflow! (file='%s')",
				       REFMON_MODNAME, name);
				break;
			}

			ret += ret;
			decrement -= ret;
		}
		SPIN_INSTANCE_UNLOCK
	} else if (MATCH_STRING(name, REFMON_FILE_SYSCALLCODES)) {
		ret = snprintf(out, sizeof(out), "%d\n%d\n%d\n%d\n",
			       syscall_codes.sc_code_set_state,
			       syscall_codes.sc_code_reconf_add,
			       syscall_codes.sc_code_reconf_rm,
			       syscall_codes.sc_code_flush);
		if (ret < 0) {
			pr_err("%s: error while trying to write to proc file (file='%s')",
			       REFMON_MODNAME, name);
			ret = -EIO;
			goto tail;
		}

	} else if (MATCH_STRING(name, REFMON_FILE_STATE)) {
		ret = snprintf(out, sizeof(out), "%s\n",
			       state_to_string(the_instance.state));
		if (ret < 0) {
			pr_err("%s: error while trying to snprintf to proc file (file='%s')",
			       REFMON_MODNAME, name);
			ret = -EIO;
			goto tail;
		}

	} else {
		pr_err("%s: invalid procpath provided - that is unexpected! (file='%s')",
		       REFMON_MODNAME, name);
		ret = -EINVAL;
		goto tail;
	}

	if (*pos > 0 || count < ret) {
		return 0;
	}

	if (copy_to_user(buf, out, ret) != 0) {
		pr_err("%s: error while trying copy_to_user (file='%s')",
		       REFMON_MODNAME, name);
		return -EFAULT;
	}
	*pos += ret;
tail:
	return ret;
}

ssize_t read_proc_syscalls(struct file *file, char __user *buf, size_t count,
			   loff_t *pos)
{
	return _read_proc(file, buf, count, pos, REFMON_FILE_SYSCALLCODES);
}

ssize_t read_proc_state(struct file *file, char __user *buf, size_t count,
			loff_t *pos)
{
	return _read_proc(file, buf, count, pos, REFMON_FILE_STATE);
}

ssize_t read_proc_list(struct file *file, char __user *buf, size_t count,
		       loff_t *pos)
{
	return _read_proc(file, buf, count, pos, REFMON_FILE_REGISTRY);
}

int init_procfiles()
{
	/*
	 * --------- Suggestion for future implementations ---------
	 * If, in the future, the number of procfiles involved starts to be
	 * more relevant, an optimization could consist of put them together
	 * in a list, maybe together in a struct with their respective handlers
	 * and whatever else may be needed by the handling.
	 * This may help in every circumstance when you have repeat the same
	 * instructions to all your procfiles, like what we are doing here.
	 *
	 * In this implementation we only have 3 procfiles, so it would have
	 * been a little an overkill to do that.
	 *
	 * Something similar can be done with kretprobes (see related files)
	 */

	struct proc_dir_entry *proc_file_entry;

	proc_file_entry = proc_create(REFMON_FILE_SYSCALLCODES, 0, NULL,
				      &procops_syscalls);
	if (proc_file_entry == NULL) {
		printk("%s: error while creating procfile entry (procfile='%s')",
		       REFMON_MODNAME, REFMON_FILE_SYSCALLCODES);
		return -EINVAL;
	}
	proc_file_entry =
		proc_create(REFMON_FILE_STATE, 0, NULL, &procops_state);
	if (proc_file_entry == NULL) {
		printk("%s: error while creating procfile entry (procfile='%s')",
		       REFMON_MODNAME, REFMON_FILE_STATE);
		return -EINVAL;
	}
	proc_file_entry =
		proc_create(REFMON_FILE_REGISTRY, 0, NULL, &procops_list);
	if (proc_file_entry == NULL) {
		printk("%s: error while creating procfile entry (procfile='%s')",
		       REFMON_MODNAME, REFMON_FILE_REGISTRY);
		return -EINVAL;
	}
	return 0;
}

void flush_procfiles(void)
{
	/*
	 * --------- Suggestion for future implementations ---------
	 * Same as init_procfiles().
	 */

	remove_proc_entry(REFMON_FILE_SYSCALLCODES, NULL);
	remove_proc_entry(REFMON_FILE_STATE, NULL);
	remove_proc_entry(REFMON_FILE_REGISTRY, NULL);
}
