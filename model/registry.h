#ifndef REFMON_REGISTRY_H
#define REFMON_REGISTRY_H

#include <linux/list.h>

struct path_identifier {
	char *path;
	unsigned long ino_no;
};

struct registry_entry {
	struct path_identifier registered;
	struct path_identifier resolved;
	struct list_head registry_noderef;
};

#endif //REFMON_REGISTRY_H
