#include <linux/version.h>
#include "probing.h"
#include "../misc/fsutils.h"
#include "../io/logio.h"
#include "../model/registry.h"
#include "../refmon.h"

int krp_hook_tail(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pr_debug("%s: tail hook", REFMON_MODNAME);
	unsigned long flags;
	spin_lock_irqsave(&the_instance.spinlock_probing, flags);
	int ret = REFMON_RETVAL_DEFAULT;
	regs_set_return_value(regs, -EACCES);

	if (regs->ax != -EACCES) {
		pr_alert(
			"%s: cannot set RAX on unhautorized access! This may lead to refmon bypassing!",
			REFMON_MODNAME);
		ret = -ENOTRECOVERABLE;
		goto tail;
	}

	ret = log_handler();
	if (ret < 0) {
		pr_err("%s: unable to schedule deffered work", REFMON_MODNAME);
		goto tail;
	}
	ret = 0;
tail:
	spin_unlock_irqrestore(&the_instance.spinlock_probing, flags);
	return ret;
}

unsigned int get_accmods(struct file *file)
{
	int flags = (int)file->f_flags;
	fmode_t mode = file->f_mode;
	unsigned int accmod = flags & O_ACCMODE;
	return accmod;
}

int is_write_acc(struct file *filestruct)
{
	unsigned int accmod = get_accmods(filestruct);
	return (accmod == O_WRONLY || accmod == O_RDWR || accmod == O_CREAT ||
		accmod == O_TRUNC || accmod == O_APPEND);
}

int krp_hook_open(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pr_debug("%s: krp hook open", REFMON_MODNAME);
	const struct path *path = (const struct path *)regs->di;
	char *dentry_path = extract_path_from_dentry(path->dentry);
	if (dentry_path == NULL) {
		pr_debug(
			"%s: path from dentry not found (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}
	char *absolute_path =
		resolve_path_alloc(dentry_path, REFMON_FSUTILS_RESOLVE_LINKS);
	if (absolute_path == NULL) {
		pr_debug(
			"%s: cannot resolve absolute path (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}

	unsigned long absolute_ino = extract_ino_no_from_path(absolute_path);

	struct path_identifier id = { absolute_path, absolute_ino };

	struct file *file = (struct file *)regs->si;
	int flags = (int)file->f_flags;
	fmode_t mode = file->f_mode;
	unsigned int acc_mode = flags & O_ACCMODE;
	if (is_write_acc(file) && is_eligible(id))
		return KRP_TO_TAILHOOK;
tail:
	return KRP_GO_AHEAD;
}
int krp_hook_rm(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pr_debug("%s: krp hook rm", REFMON_MODNAME);
	struct dentry *dentry;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	dentry = (struct dentry *)regs->dx;
#else
	dentry = (struct dentry *)regs->si;
#endif

	char *pfd = extract_path_from_dentry(dentry);
	if (pfd == NULL) {
		pr_debug(
			"%s: path from dentry not found (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}

	char *absolute_path =
		resolve_path_alloc(pfd, REFMON_FSUTILS_RESOLVE_LINKS);
	if (absolute_path == NULL) {
		pr_debug(
			"%s: cannot resolve absolute path (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}

	unsigned long absolute_ino = extract_ino_no_from_path(absolute_path);
	if (absolute_ino == REFMON_NO_INO_NO) {
		pr_debug("%s: cannot extract inode number", REFMON_MODNAME);
		goto tail;
	}

	struct path_identifier id = { absolute_path, absolute_ino };
	if (is_eligible(id))

		return KRP_TO_TAILHOOK;

tail:
	return KRP_GO_AHEAD;
}

int krp_hook_mkdir(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pr_debug("%s: krp hook mkdir", REFMON_MODNAME);

	struct dentry *dentry;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	dentry = (struct dentry *)regs->dx;
#else
	dentry = (struct dentry *)regs->si;
#endif

	char *pfd = extract_path_from_dentry(dentry);
	if (pfd == NULL) {
		pr_debug(
			"%s: path from dentry not found (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}

	char *absolute_path = resolve_path_alloc(
		pfd, REFMON_FSUTILS_RESOLVE_LINKS |
			     REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS);
	if (absolute_path == NULL) {
		pr_debug(
			"%s: cannot resolve absolute path (it is not necessary an error)",
			REFMON_MODNAME);
		goto tail;
	}

	struct path_identifier id = { absolute_path, REFMON_NO_INO_NO };

	if (is_eligible(id) == 1) {
		return KRP_TO_TAILHOOK;
	}
tail:
	return KRP_GO_AHEAD;
}

struct kretprobe krp_vfs_open = { .kp.symbol_name = "vfs_open",
				  .entry_handler =
					  (kretprobe_handler_t)krp_hook_open,
				  .handler = (kretprobe_handler_t)krp_hook_tail,
				  .maxactive = REFMON_PROBES_NO,
				  .kp.flags = KPROBE_FLAG_DISABLED };

struct kretprobe krp_vfs_unlink = {
	.kp.symbol_name = "may_delete",
	.entry_handler = (kretprobe_handler_t)krp_hook_rm,
	.handler = (kretprobe_handler_t)krp_hook_tail,
	.maxactive = REFMON_PROBES_NO,
	.kp.flags = KPROBE_FLAG_DISABLED
};

struct kretprobe krp_vfs_mkdir = {
	.kp.symbol_name = "security_path_mkdir",
	.entry_handler = (kretprobe_handler_t)krp_hook_mkdir,
	.handler = (kretprobe_handler_t)krp_hook_tail,
	.maxactive = REFMON_PROBES_NO,
	.kp.flags = KPROBE_FLAG_DISABLED
};

struct kretprobe krp_vfs_rmdir = {
	.kp.symbol_name = "security_path_rmdir",
	.entry_handler = (kretprobe_handler_t)krp_hook_mkdir,
	.handler = (kretprobe_handler_t)krp_hook_tail,
	.maxactive = REFMON_PROBES_NO,
	.kp.flags = KPROBE_FLAG_DISABLED
};
