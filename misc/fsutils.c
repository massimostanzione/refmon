#include <linux/syscalls.h>
#include <linux/fs_struct.h>
#include <linux/namei.h>
#include "fsutils.h"
#include "../model/instance.h"
#include "../model/registry.h"
#include "../refmon.h"
#include "misc.h"

int is_subpath(const char *base_path, const char *sub_path)
{
	return strncmp(base_path, sub_path, strlen(base_path)) == 0 &&
	       (sub_path[strlen(base_path)] == '\0' ||
		sub_path[strlen(base_path)] == '/');
}

int is_dir(const char *path)
{
	struct path p;
	int ret;

	ret = kern_path(path, LOOKUP_FOLLOW, &p);
	if (ret != 0) {
		pr_err("%s: error while trying to check if path is a directory (path='%s')",
		       REFMON_MODNAME, path);
		ret = -1;
		goto tail;
	}

	ret = S_ISDIR(EXTRACT_INODE(p)->i_mode) ? 1 : 0;
	path_put(&p);
tail:
	return ret;
}

struct registry_entry *search_entry(struct path_identifier *id,
				    int search_criteria)
{
	struct registry_entry *curr_node;
	
	if (list_empty(&the_instance.registry)) {
		pr_info("%s: list is empty, node search skipped. (node.path='%s', node.ino_no=%lu).",
			REFMON_MODNAME, id->path, id->ino_no);
		goto tail_notfound;
	}

	if (id->path == NULL) {
		pr_err("%s: provided a NULL path to be searched into the refmon registry.",
		       REFMON_MODNAME);
		goto tail_notfound;
	}

	if (MATCH_STRING(id->path, "") &&
	    (search_criteria & REFMON_SEARCH_CRIT_REGISTERED_PATH ||
	     search_criteria & REFMON_SEARCH_CRIT_ABSOLUTE_PATH ||
	     search_criteria & REFMON_SEARCH_CRIT_SUBDIR_CHECK)) {
		pr_err("%s: search criteria with path involved, but no path provided. (criteria=%d).",
		       REFMON_MODNAME, search_criteria);
		goto tail_notfound;
	}

	list_for_each_entry(curr_node, &the_instance.registry,
			    registry_noderef) {
		if (((search_criteria & REFMON_SEARCH_CRIT_REGISTERED_PATH) &&
		     MATCH_STRING(curr_node->registered.path, id->path)) ||
		    ((search_criteria & REFMON_SEARCH_CRIT_REGISTERED_INO) &&
		     curr_node->registered.ino_no == id->ino_no) ||
		    ((search_criteria & REFMON_SEARCH_CRIT_ABSOLUTE_PATH) &&
		     MATCH_STRING(curr_node->resolved.path, id->path)) ||
		    ((search_criteria & REFMON_SEARCH_CRIT_ABSOLUTE_INO) &&
		     curr_node->resolved.ino_no == id->ino_no) ||
		    ((search_criteria & REFMON_SEARCH_CRIT_SUBDIR_CHECK) &&
		     (is_dir(curr_node->resolved.path) &&
		      is_subpath(curr_node->resolved.path, id->path))))
			return curr_node;
	}
tail_notfound:
	return NULL;
}

int is_resolved(const char *path)
{
	return path[0] == '/';
}

char *resolve_path_alloc(const char *input_path, int flags)
{
	char *ret_path, *temp, *working_copy;
	struct path path;
	int ret = REFMON_RETVAL_DEFAULT;

	if (input_path == NULL || strlen(input_path) == 0) {
		pr_debug(
			"%s: input path is empty, avoiding useless further processing",
			REFMON_MODNAME);
		goto tail_err;
	}

	working_copy = kstrdup(input_path, GFP_KERNEL);
	if (working_copy == NULL) {
		pr_err("%s: error while duplicating input path string",
		       REFMON_MODNAME);
		goto tail_err;
	}

	if ((flags & REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS) &&
	    is_resolved(working_copy)) {
		ret_path = working_copy;
		goto tail;
	}

	ret_path = kmalloc(PATH_MAX * sizeof(char), GFP_KERNEL);
	if (ret_path == NULL) {
		pr_err("%s: error while trying to allocate memory",
		       REFMON_MODNAME);
		goto tail_err;
	}

	if (flags & REFMON_FSUTILS_RESOLVE_LINKS) {
		ret = kern_path(working_copy, LOOKUP_FOLLOW, &path);
		if (ret != 0) {
			pr_debug("%s: cannot do kern_path",
			       REFMON_MODNAME);
			goto tail_err;
		}

		temp = dentry_path_raw(path.dentry, ret_path, PATH_MAX);
		if (IS_ERR(temp)) {
			pr_err("%s: error while resolving complete path (dentry_path_raw)",
			       REFMON_MODNAME);
			path_put(&path);
			goto tail_err;
		}

		if (strlen(temp) >= PATH_MAX) {
			pr_err("%s: resolve_path is too long (resolved_path='%s')",
			       REFMON_MODNAME, temp);
			path_put(&path);
			goto tail_err;
		}

		path_put(&path);
		ret_path = temp;
		goto tail;
	} else {
		path = current->fs->pwd;
		temp = d_path(&path, ret_path, PATH_MAX);
		if (IS_ERR(temp)) {
			pr_err("%s: error while trying to get current abs path",
			       REFMON_MODNAME);
			goto tail_err;
		}

		ret = snprintf(ret_path, PATH_MAX, "%s/%s", temp, working_copy);
		if (ret < 0 || ret >= PATH_MAX) {
			pr_err("%s: resolve_path is too long (resolved_path='%s')",
			       REFMON_MODNAME, temp);
			goto tail_err;
		}
		goto tail;
	}
tail_err:
	ret_path = NULL;
tail:
	return ret_path;
}

int is_eligible(struct path_identifier id)
{
	struct registry_entry *ret;

	ret = search_entry(&id, REFMON_SEARCH_CRIT_ABSOLUTE_PATH |
					REFMON_SEARCH_CRIT_ABSOLUTE_INO |
					REFMON_SEARCH_CRIT_SUBDIR_CHECK);
	return ret == NULL ? 0 : 1;
}

unsigned long extract_ino_no_from_path(const char *path)
{
	struct path p;
	int ret = REFMON_RETVAL_DEFAULT;
	ret = kern_path(path, 0, &p);
	if (ret != 0) {
		pr_err("%s: error while trying to execute kern_path (path='%s')",
		       REFMON_MODNAME, path);
		return REFMON_NO_INO_NO;
	}
	return EXTRACT_INODE(p)->i_ino;
}

char *extract_path_from_dentry(struct dentry *dentry)
{
	int len;
	char *buffer, *ret = NULL;

	buffer = kmalloc(PATH_MAX * sizeof(char), GFP_ATOMIC);
	if (buffer == NULL) {
		pr_err("%s: error while trying kmalloc", REFMON_MODNAME);
		goto tail;
	}

	ret = dentry_path_raw(dentry, buffer, PATH_MAX);
	if (IS_ERR(ret)) {
		pr_err("%s: error while trying dentry_path_raw",
		       REFMON_MODNAME);
		goto tail;
	}

	len = strlen(ret);
	if (len >= PATH_MAX) {
		pr_err("%s: pathname is too long (path='%s')", REFMON_MODNAME,
		       ret);
		goto tail;
	}
tail:
	FREE(buffer);
	return ret == NULL ? ret : strcat(ret, "\0");
}

struct registry_entry *find_entry_by_regpath_alloc(const char *registered_path)
{
	struct registry_entry *ret = NULL;
	struct path_identifier id;
	char *solved = resolve_path_alloc(registered_path,
					  REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS);
	if (solved == NULL) {
		pr_err("%s: error while trying to resolve path (path='%s')",
		       REFMON_MODNAME, registered_path);
		goto tail;
	}

	id.path = solved;
	id.ino_no = REFMON_NO_INO_NO;
	
	ret = search_entry(&id, REFMON_SEARCH_CRIT_REGISTERED_PATH);
	if (ret == NULL) {
		pr_err("%s: error while trying to search entry for id (id.path='%s', id.ino=%ld)",
		       REFMON_MODNAME, id.path, id.ino_no);
		goto tail;
	}
tail:
	return ret;
}
