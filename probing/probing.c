#include <linux/version.h>
#include "probing.h"
#include "../misc/fsutils.h"
#include "../io/logio.h"
#include "../model/registry.h"
#include "../refmon.h"

int probing_off(void)
{
	pr_debug("%s: trying to STOP probing...", REFMON_MODNAME);
	int ret = REFMON_RETVAL_DEFAULT;

	disable_kretprobe(&krp_vfs_open);
	disable_kretprobe(&krp_vfs_unlink);
	disable_kretprobe(&krp_vfs_mkdir);
	disable_kretprobe(&krp_vfs_rmdir);
	pr_debug("%s: probing OFF", REFMON_MODNAME);
	ret = 0;
tail:
	pr_cont(" done");
	return ret;
}

int probing_on(void)
{
	pr_debug("%s: trying to start probing...", REFMON_MODNAME);
	int ret = REFMON_RETVAL_DEFAULT;

	enable_kretprobe(&krp_vfs_open);
	enable_kretprobe(&krp_vfs_unlink);
	enable_kretprobe(&krp_vfs_mkdir);
	enable_kretprobe(&krp_vfs_rmdir);

	pr_cont(" done");
	ret = 0;

tail:
	return ret;
}

int probing_update(int desired_probing_state)
{
	return desired_probing_state ? probing_on() : probing_off();
}

int init_probes(void)
{
	/*
	* --------- Suggestion for future implementations ---------
	* If, in the future, the number of kretprobes involved starts to be
	* more relevant, an optimization could consist of put them together
	* in a list, maybe together in a struct with their respective handlers
	* and whatever else may be needed by the handling.
	* This may help in every circumstance when you have repeat the same
	* instructions to all your kretprobes, like what we are doing here.
	*
	* In this implementation we only have 6 kretprobes, so it would have
	* been a little an overkill to do that.
	*
	* Something similar can be done with procfiles (see related files)
	*/

	int ret = REFMON_RETVAL_DEFAULT;

	ret = register_kretprobe(&krp_vfs_open);
	if (ret < 0) {
		pr_err("%s: error while trying to register kretprobe handler(probed=OPEN, ret=%d)",
		       REFMON_MODNAME, ret);
		goto tail;
	}

	ret = register_kretprobe(&krp_vfs_unlink);
	if (ret < 0) {
		pr_err("%s: error while trying to register kretprobe handler(probed=UNLINK, ret=%d)",
		       REFMON_MODNAME, ret);
		goto tail;
	}

	ret = register_kretprobe(&krp_vfs_mkdir);
	if (ret < 0) {
		pr_err("%s: error while trying to register kretprobe handler(probed=MKDIR, ret=%d)",
		       REFMON_MODNAME, ret);
		goto tail;
	}

	ret = register_kretprobe(&krp_vfs_rmdir);
	if (ret < 0) {
		pr_err("%s: error while trying to register kretprobe handler(probed=RMDIR, ret=%d)",
		       REFMON_MODNAME, ret);
		goto tail;
	}

	pr_debug("%s: kretprobes successifully registered", REFMON_MODNAME);
	ret = 0;
tail:
	return ret;
}

void flush_probes(void)
{
	/*
	 * --------- Suggestion for future implementations ---------
	 * Same as init_probes().
	 */

	unregister_kretprobe(&krp_vfs_open);
	unregister_kretprobe(&krp_vfs_unlink);
	unregister_kretprobe(&krp_vfs_mkdir);
	unregister_kretprobe(&krp_vfs_rmdir);

	pr_debug("%s: kretprobes successifully unregistered", REFMON_MODNAME);
}
