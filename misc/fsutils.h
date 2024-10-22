#ifndef REFMON_FSUTILS_H
#define REFMON_FSUTILS_H

#include "../model/registry.h"

#define REFMON_SEARCH_CRIT_REGISTERED_PATH 0x01
#define REFMON_SEARCH_CRIT_REGISTERED_INO 0x02
#define REFMON_SEARCH_CRIT_ABSOLUTE_PATH 0x04
#define REFMON_SEARCH_CRIT_ABSOLUTE_INO 0x08
#define REFMON_SEARCH_CRIT_SUBDIR_CHECK 0x10

#define REFMON_FSUTILS_RESOLVE_LINKS 0x01
#define REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS 0x02
#define REFMON_FSUTILS_NONE 0x00

#define EXTRACT_INODE(p) p.dentry->d_inode

#define REFMON_NO_PATH "--"
#define REFMON_NO_INO_NO -1

char *resolve_path_alloc(const char *input_path, int flags);

int is_eligible(struct path_identifier id);

struct registry_entry *find_entry_by_regpath_alloc(const char *registered_path);

char *extract_path_from_dentry(struct dentry *dentry);

unsigned long extract_ino_no_from_path(const char *path);

#endif //REFMON_FSUTILS_H
