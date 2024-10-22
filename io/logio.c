#include <linux/spinlock.h>
#include "logio.h"
#include "files.h"
#include "../misc/fsutils.h"
#include "../misc/misc.h"
#include "../refmon.h"

spinlock_t spin;

void defer(struct work_struct *work)
{
	struct log_dataset *log_dataset_inst;
	char out[REFMON_LOGLEN], *hash;
	struct file *file;
	ssize_t outb;
	
	spin_lock(&spin);
	log_dataset_inst =
		container_of(work, struct deferred_work, work)->log_dataset;

	hash = hashgen_filecont(log_dataset_inst->prog_path);
	if (hash == NULL) {
		pr_err("%s: error while trying to generate file hash",
		       REFMON_MODNAME);
		goto tail;
	}

	snprintf(out, sizeof(out), "%d\t%d\t%u\t%u\t%s\t%s",
		 log_dataset_inst->tgid, log_dataset_inst->tid,
		 log_dataset_inst->uid, log_dataset_inst->euid,
		 log_dataset_inst->prog_path, hash);

	file = filp_open(REFMON_FILE_LOG_ABSOLUTE,
				      O_WRONLY | O_CREAT | O_APPEND,
				      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (IS_ERR(file)) {
		pr_err("%s: error while trying to open log file",
		       REFMON_MODNAME);
		goto tail;
	}

	outb = kernel_write(file, out, strlen(out), &file->f_pos);
	if (outb < 0) {
		pr_err("%s: overflow while writing towards log file",
		       REFMON_MODNAME);
		goto tail;
	}
	filp_close(file, NULL);

tail:

	spin_unlock(&spin);
	FREE(hash);
}

int log_handler(void)
{
	int ret = REFMON_RETVAL_DEFAULT;
	struct log_dataset *log_dataset_inst = NULL;
	struct deferred_work *log_twork = NULL;
	char *path = NULL;

	log_dataset_inst = kmalloc(sizeof(struct log_dataset), GFP_ATOMIC);
	if (log_dataset_inst == NULL) {
		pr_err("%s: error while trying to kmalloc log_dataset",
		       REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	path = extract_path_from_dentry(
		current->mm->exe_file->f_path.dentry);
	if (path == NULL) {
		pr_err("%s: error while trying to extract path from dentry",
		       REFMON_MODNAME);
		ret = -EINVAL;
		goto tail;
	}

	log_dataset_inst->tgid = task_tgid_vnr(current);
	log_dataset_inst->tid = current->pid;
	log_dataset_inst->uid = current_uid().val;
	log_dataset_inst->euid = current_euid().val;

	log_dataset_inst->prog_path = kstrdup(path, GFP_ATOMIC);
	if (log_dataset_inst->prog_path == NULL) {
		pr_err("%s: error while trying to duplicate path",
		       REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	log_twork = kmalloc(sizeof(struct deferred_work), GFP_ATOMIC);
	if (log_twork == NULL) {
		pr_err("%s: error while trying to kmalloc work task",
		       REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	log_twork->buffer = log_twork;
	log_twork->log_dataset = log_dataset_inst;

	INIT_WORK(&(log_twork->work), defer);
	schedule_work(&log_twork->work);
	ret = 0;

tail:
	return ret;
}
