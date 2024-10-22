#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/cred.h>
#include "safety.h"
#include "../model/instance.h"
#include "../refmon.h"
#include "../misc/misc.h"

int is_euid_root(void)
{
	kuid_t euid;
	euid = current_euid();
	if (euid.val < 0) {
		pr_err("%s: could not retrieve current euid (ret=%d)",
		       REFMON_MODNAME, euid);
		return euid.val;
	}
	return uid_eq(euid, GLOBAL_ROOT_UID);
}

/**
 * sanitize_user_input_alloc() - Sanitize (i.e., make a kernel copy) some user input string
 * @unsafe_input: user "unsafe" input
 *
 * Context: allocates dynamic memory (should be kfree-d when no longer useful).
 *
 * Return: "safe", sanitized output string, or NULL if some error occours
 */
char *sanitize_user_input_alloc(char __user *unsafe_input)
{
	char *safe_output;
	int outsize = strlen(unsafe_input) + 1;

	if (unsafe_input == NULL || MATCH_STRING(unsafe_input, "")) {
		pr_info("%s: input is NULL or empty, impossible to sanitize",
			REFMON_MODNAME);
		goto tail_err;
	}

	safe_output = kmalloc(outsize, GFP_ATOMIC);
	if (safe_output == NULL) {
		pr_err("%s: error while trying to allocate memory",
		       REFMON_MODNAME);
		goto tail_err;
	}

	if (copy_from_user(safe_output, unsafe_input, outsize)) {
		pr_err("%s: error while trying copy_from_user", REFMON_MODNAME);
		kfree(safe_output);
		goto tail_err;
	}
	goto tail;
tail_err:
	safe_output = NULL;
tail:
	return safe_output;
}

int check_reconf(void)
{
	if (!is_reconf()) {
		pr_info("%s: not in a reconfigurable state (%s), blocking the requested operation",
			REFMON_MODNAME, state_to_string(the_instance.state));
		return -1;
	}
	return 0;
}

int safety_checks(int checks, char *arg)
{
	int ret = REFMON_RETVAL_DEFAULT, caught = 0, is_root = REFMON_RETVAL_DEFAULT;
	char *safe_password = NULL;
	
	if (checks & REFMON_SAFETY_CHECK_NONE) {
		pr_debug("%s: SAFETY CHECK - none requested", REFMON_MODNAME);
		ret = 0;
		goto tail;
	}
	if (checks & REFMON_SAFETY_CHECK_ROOT) {
		pr_debug("%s: SAFETY CHECK - root (euid)", REFMON_MODNAME);
		caught++;
		is_root = is_euid_root();
		if (is_root < 0) {
			ret = -EINVAL;
			goto tail;
		}
		if (!is_root) {
			ret = -EACCES;
			goto tail;
		}
	}
	if (checks & REFMON_SAFETY_CHECK_RECONF) {
		caught++;
		pr_debug("%s: SAFETY CHECK - reconfigurable state",
			 REFMON_MODNAME);
		if (check_reconf() != 0) {
			ret = -EPERM;
			goto tail;
		}
	}
	if (checks & REFMON_SAFETY_CHECK_PASSWORD) {
		pr_debug("%s: SAFETY CHECK - password match", REFMON_MODNAME);
		caught++;

		safe_password = sanitize_user_input_alloc(arg);
		if (safe_password == NULL) {
			pr_err("Cannot sanitize user password");
			ret = -EINVAL;
			goto tail;
		}

		if (!MATCH_STRING(hashgen_str(safe_password),
				  the_instance.password)) {
			pr_err("Wrong password");
			ret = -EACCES;
			goto tail;
		}
	}
	if (caught == 0) {
		pr_err("%s: unrecognized check requested (%d) - it is not safe to go ahead, blocking",
		       REFMON_MODNAME, checks);

		ret = -ENOTRECOVERABLE;
		goto tail;
	}
	ret = 0;
tail:
	FREE(safe_password);
	return ret;
}
