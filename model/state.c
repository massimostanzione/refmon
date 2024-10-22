#include <linux/module.h>
#include "instance.h"
#include "state.h"
#include "../refmon.h"
#include "../misc/misc.h"

enum refmon_state string_to_state(char *state)
{
	if (MATCH_STRING(state, REFMON_STATE_STRING_OFF))
		return OFF;
	if (MATCH_STRING(state, REFMON_STATE_STRING_ON))
		return ON;
	if (MATCH_STRING(state, REFMON_STATE_STRING_RECOFF))
		return REC_OFF;
	if (MATCH_STRING(state, REFMON_STATE_STRING_RECON))
		return REC_ON;
	pr_err("%s: invalid state value provided (state='%s')", REFMON_MODNAME,
	       state);
	return -EINVAL;
}

char *state_to_string(enum refmon_state state)
{
	if (state == OFF)
		return REFMON_STATE_STRING_OFF;
	if (state == ON)
		return REFMON_STATE_STRING_ON;
	if (state == REC_OFF)
		return REFMON_STATE_STRING_RECOFF;
	if (state == REC_ON)
		return REFMON_STATE_STRING_RECON;
	pr_err("%s: invalid state value provided (state='%s')", REFMON_MODNAME,
	       state);
	return NULL;
}

int is_state_valid(char *state)
{
	return MATCH_STRING(state, REFMON_STATE_STRING_OFF) ||
	       MATCH_STRING(state, REFMON_STATE_STRING_ON) ||
	       MATCH_STRING(state, REFMON_STATE_STRING_RECOFF) ||
	       MATCH_STRING(state, REFMON_STATE_STRING_RECON);
}
