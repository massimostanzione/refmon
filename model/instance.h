#ifndef REFMON_INSTANCE_H
#define REFMON_INSTANCE_H

#include <linux/spinlock.h>
#include "registry.h"
#include "state.h"

#define SPIN_INSTANCE_LOCK spin_lock(&the_instance.spinlock_inst);
#define SPIN_INSTANCE_UNLOCK spin_unlock(&the_instance.spinlock_inst);

typedef struct refmon {
	enum refmon_state state;
	struct list_head registry;
	char *password;
	spinlock_t spinlock_inst;
	spinlock_t spinlock_probing;
};

extern struct refmon the_instance;

extern int is_off(void);
extern int is_on(void);
extern int is_reconf(void);

#endif //REFMON_INSTANCE_H
