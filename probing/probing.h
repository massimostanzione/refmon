#ifndef REFMON_PROBING_H
#define REFMON_PROBING_H

#include <linux/kprobes.h>

#define REFMON_PROBES_NO 15
#define KRP_TO_TAILHOOK 0
#define KRP_GO_AHEAD 1

#define PROBING_STATE_OFF 0
#define PROBING_STATE_ON 1
extern int probing_state;

extern struct kretprobe krp_vfs_open, krp_vfs_write, krp_vfs_unlink,
	krp_vfs_mkdir, krp_vfs_rmdir, krp_vfs_fsync, krp_sec_ino_create;

 int krp_hook_open(struct kretprobe_instance *ri, struct pt_regs *regs);
 int krp_hook_rm(struct kretprobe_instance *ri, struct pt_regs *regs);
 int krp_hook_tail(struct kretprobe_instance *ri, struct pt_regs *regs);
 int krp_hook_mkdir(struct kretprobe_instance *ri, struct pt_regs *regs);

int probing_update(int desired_probing_state);
int init_probes(void);
void flush_probes(void);

#endif //REFMON_PROBING_H
