#ifndef REFMON_PROC_H
#define REFMON_PROC_H

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

#include "../misc/kernelvers_limits.h"

#if LINUX_VERSION_CODE < REFMON_KERNELVER_PROCMGT
extern struct file_operations procops_syscalls;
extern struct file_operations procops_state;
extern struct file_operations procops_list;
#else
extern struct proc_ops procops_syscalls;
extern struct proc_ops procops_state;
extern struct proc_ops procops_list;
#endif

ssize_t read_proc_syscalls(struct file *filp, char *buf, size_t count,
			   loff_t *offp);
ssize_t read_proc_state(struct file *filp, char *buf, size_t count,
			loff_t *offp);
ssize_t read_proc_list(struct file *filp, char *buf, size_t count,
		       loff_t *offp);

int init_procfiles(void);
void flush_procfiles(void);

#endif //REFMON_PROC_H
