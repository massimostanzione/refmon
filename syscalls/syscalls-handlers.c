#include "syscalls.h"
#include "../probing/probing.h"
#include "../model/instance.h"
#include "../model/state.h"
#include "../model/registry.h"
#include "../misc/fsutils.h"
#include "../refmon.h"
#include "../misc/misc.h"
#include "safety.h"

int _set_state(char *new_state)
{
	int ret = REFMON_RETVAL_DEFAULT;
	the_instance.state = string_to_state(new_state);
	ret = probing_update(
		is_on()); // il controllo lo faccio sullo stato già cambiato
	if (ret != 0) {
		pr_err("%s: error while updating probing state with state change - inconsistences may be present! (new_state=%s, probing_state=TODO)",
		       REFMON_MODNAME, new_state);
		goto tail;
	}
	ret = 0;
tail:
	return ret;
}

int _add_file(char *path)
{
	int ret = REFMON_RETVAL_DEFAULT;
	char *registered_path, *resolved_path;
	unsigned long ino_registered, ino_resolved;
	struct registry_entry *node;
	struct path_identifier registered, resolved;
	
	registered_path = kmalloc(PATH_MAX * sizeof(char), GFP_ATOMIC);
	if (registered_path == NULL) {
		pr_err("%s: error while trying kmalloc", REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	registered_path =
		resolve_path_alloc(path, REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS);

	if (registered_path == NULL) {
		pr_err("%s: error while trying to resolve path (path='%s')",
		       REFMON_MODNAME, path);
		ret = -EINVAL;
		goto tail;
	}

	if (find_entry_by_regpath_alloc(registered_path) != NULL) {
		pr_err("%s: path is already registered (path='%s')",
		       REFMON_MODNAME, path);
		ret = -EEXIST;
		goto tail;
	}

	resolved_path = kmalloc(PATH_MAX * sizeof(char), GFP_ATOMIC);
	if (resolved_path == NULL) {
		pr_err("%s: error while trying kmalloc", REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	resolved_path = resolve_path_alloc(
		path, REFMON_FSUTILS_RESOLVE_LINKS |
			      REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS);
	if (resolved_path == NULL) {
		pr_err("%s: error while trying to compute resolved path (path='%s')",
		       REFMON_MODNAME, path);
		ret = -EINVAL;
		goto tail;
	}

	ino_registered = extract_ino_no_from_path(path);
	if (ino_registered == REFMON_NO_INO_NO) {
		pr_err("%s: error while trying to extract inode number - maybe file does not exist (path='%s')",
		       REFMON_MODNAME, path);
		ret = -EINVAL;
		goto tail;
	}

	ino_resolved = extract_ino_no_from_path(resolved_path);
	if (ino_resolved == REFMON_NO_INO_NO) {
		pr_err("%s: error while trying to extract inode number - maybe file does not exist (path='%s')",
		       REFMON_MODNAME, path);
		ret = -EINVAL;
		goto tail;
	}

	node = kmalloc(sizeof(struct registry_entry), GFP_ATOMIC);
	if (node == NULL) {
		pr_err("%s: error while trying kmalloc for registry node",
		       REFMON_MODNAME);
		ret = -ENOMEM;
		goto tail;
	}

	registered.path = registered_path;
	registered.ino_no = ino_registered;
	
	resolved.path = resolved_path;
	resolved.ino_no = ino_resolved;
	
	node->registered = registered;
	node->resolved = resolved;

	list_add_tail(&node->registry_noderef, &the_instance.registry);
	ret = 0;
tail:
	return ret;
}

int do_set_state(char __user *new_state)
{
	int ret = REFMON_RETVAL_DEFAULT;
	SPIN_INSTANCE_LOCK
	pr_debug("%s: _set_state, param: %s", REFMON_MODNAME, new_state);

	ret = safety_checks(REFMON_SAFETY_CHECK_ROOT, NULL);
	if (ret != 0)
		goto tail;

	if (!is_state_valid(new_state)) {
		ret = -EINVAL;
		goto tail;
	}

	ret = _set_state(new_state);
	if (ret != 0) {
		pr_err("%s: error while trying to set new state (new_state=%s)",
		       REFMON_MODNAME, new_state);
		goto tail;
	}
	ret = 0;
tail:
	SPIN_INSTANCE_UNLOCK
	return ret;
}

int do_reconf_add(char __user *path_in, char __user *password_in)
{
	int ret = REFMON_RETVAL_DEFAULT;
	char *safe_path;
	
	SPIN_INSTANCE_LOCK
	
	pr_debug("%s: do_reconf_add, params: %s %s", REFMON_MODNAME, path_in,
		 password_in);
		 
	ret = safety_checks(REFMON_SAFETY_CHECK_ROOT |
				    REFMON_SAFETY_CHECK_RECONF |
				    REFMON_SAFETY_CHECK_PASSWORD,
			    password_in);
			    
	if (ret != 0)
		goto tail;

	safe_path = sanitize_user_input_alloc(path_in);
	if (safe_path == NULL) {
		pr_err("%s: cannot sanitize path, aborting", REFMON_MODNAME);
		ret = -EINVAL;
		goto tail;
	}

	ret = _add_file(safe_path);
	if (ret != 0) {
		pr_err("%s: error while trying to add file (path='%s')",
		       REFMON_MODNAME, safe_path);
		goto tail;
	}
	pr_info("%s: path added (path='%s')", REFMON_MODNAME, safe_path);
	ret = 0;
tail:
	SPIN_INSTANCE_UNLOCK
	return ret;
}

int do_reconf_rm(char __user *path_in, char __user *password_in)
{
	char *safe_path = NULL;
	int ret = REFMON_RETVAL_DEFAULT;
	struct registry_entry *found;

	SPIN_INSTANCE_LOCK

	pr_debug("%s: do_reconf_rm, params: %s %s", REFMON_MODNAME, path_in,
		 password_in);
	ret = safety_checks(REFMON_SAFETY_CHECK_ROOT |
				    REFMON_SAFETY_CHECK_RECONF |
				    REFMON_SAFETY_CHECK_PASSWORD,
			    password_in);
			    
	if (ret != 0)
		goto tail;
		
	safe_path = sanitize_user_input_alloc(path_in);
	if (safe_path == NULL) {
		pr_err("%s: cannot sanitize path, aborting", REFMON_MODNAME);
		ret = -EINVAL;
		goto tail;
	}

	found = find_entry_by_regpath_alloc(
		resolve_path_alloc(safe_path,
				   REFMON_FSUTILS_IGNORE_ABSOLUTE_PATHS));
	if (found == NULL) {
		pr_err("%s: path requested for removal is not registered (path='%s')",
		       REFMON_MODNAME, safe_path);
		ret = -ENOENT;
		goto tail;
	}

	list_del(&found->registry_noderef);

	pr_info("%s: removed path %s", REFMON_MODNAME, safe_path);
	ret = 0;
tail:
	FREE(safe_path);
	SPIN_INSTANCE_UNLOCK
	return ret;
}

// enabled only with -DDEBUG flag
int do_flush_registry(char __user *password)
{
	int ret = REFMON_RETVAL_DEFAULT;
	struct registry_entry *node, *node_aux;
	SPIN_INSTANCE_LOCK
	pr_debug("%s: do_flush_registry, params: %s", REFMON_MODNAME, password);
	ret = safety_checks(REFMON_SAFETY_CHECK_ROOT |
				    REFMON_SAFETY_CHECK_RECONF |
				    REFMON_SAFETY_CHECK_PASSWORD,
			    password);
			    
	if (ret != 0)
		goto tail;

	if (list_empty(&the_instance.registry)) {
		pr_info("%s: empty list, skipping...", REFMON_MODNAME);
		ret = 0;
		goto tail;
	}

	list_for_each_entry_safe(node, node_aux, &the_instance.registry,
				 registry_noderef) {
		pr_debug("%s: FLUSH - deleting %s", REFMON_MODNAME,
			 node->registered.path);
		list_del(&node->registry_noderef);
	}
	
	pr_info("%s: register flushed", REFMON_MODNAME);
	ret = 0;
tail:
	SPIN_INSTANCE_UNLOCK
	return ret;
}
