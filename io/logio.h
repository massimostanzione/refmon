#ifndef REFMON_LOGIO_H
#define REFMON_LOGIO_H

#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <crypto/hash.h>

#define REFMON_LOGLEN 516

extern struct log_dataset {
	pid_t tgid;
	pid_t tid;
	uid_t uid;
	uid_t euid;
	char *prog_path;
} log_dataset;

extern struct deferred_work {
	void *buffer;
	struct log_dataset *log_dataset;
	struct work_struct work;
} deferred_work;

extern int log_handler(void);

#endif //REFMON_LOGIO_H
