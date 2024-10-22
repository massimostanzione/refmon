#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

static char *strings_states_shorts[] = { "--recon", "--on", "--off", "--recoff",
					 NULL };

static MunitParameterEnum params_states_shorts[] = {
	{ "sudo", strings_sudo },
	{ "short", strings_states_shorts },
	{ NULL, NULL },
};

static char *short_to_string(const char *shorts)
{
	if (MATCH_STRING(shorts, "--off"))
		return "OFF";
	if (MATCH_STRING(shorts, "--on"))
		return "ON";
	if (MATCH_STRING(shorts, "--recoff"))
		return "REC-OFF";
	if (MATCH_STRING(shorts, "--recon"))
		return "REC-ON";
	return NULL;
}

static MunitResult test_set_state_valid(const MunitParameter params[],
					void *fixture)
{
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *state = munit_parameters_get(params, "state");
	int ret = test_exec(sudo, TEST_PREAMBLE_SET_STATE, state);
	munit_assert_int(ret, ==, TEST_IS_SUDO ? 0 : -EACCES);
	if (TEST_IS_SUDO) {
		char out[TEST_BSIZE];
		ret = test_exec_output("", TEST_COMMAND_PRINT_STATE, "", out);
		munit_assert_int(ret, ==, 0);

		int occ = test_string_occurrences(out, state);
		munit_assert_int(occ, ==, 1);
	}
	return MUNIT_OK;
}

static MunitResult test_set_state_invalid(const MunitParameter params[],
					  void *fixture)
{
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *state = munit_parameters_get(params, "invalid");
	int ret = test_exec(sudo, TEST_PREAMBLE_SET_STATE, state);
	int multi_param = test_string_occurrences(state, " ") > 0;
	if (multi_param || MATCH_STRING(state, ""))
		munit_assert_int(ret, ==, -EINVAL);
	else
		munit_assert_int(ret, ==, TEST_IS_SUDO ? -EINVAL : -EACCES);

	// check that invalid params are not inserted into the program
	if (TEST_IS_SUDO || multi_param) {
		char out[TEST_BSIZE];
		ret = test_exec_output("", TEST_COMMAND_PRINT_STATE, "", out);
		munit_assert_int(ret, ==, 0);
		int occ = test_string_occurrences(out, state);
		munit_assert_int(occ, ==, 0);
	}
	return MUNIT_OK;
}

static MunitResult test_set_state_shorts(const MunitParameter params[],
					 void *fixture)
{
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *shorts = munit_parameters_get(params, "short");
	int ret = test_exec(sudo, TEST_PREAMBLE, shorts);
	munit_assert_int(ret, ==, TEST_IS_SUDO ? 0 : -EACCES);
	if (TEST_IS_SUDO) {
		char out[TEST_BSIZE];
		ret = test_exec_output("", TEST_COMMAND_PRINT_STATE, "", out);
		munit_assert_int(ret, ==, 0);

		int occ = test_string_occurrences(out, short_to_string(shorts));
		munit_assert_int(occ, ==, 1);
	}
	return MUNIT_OK;
}

static MunitTest ts_set_state_tests[] = {
	{ "valid", test_set_state_valid, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_states_valid },
	{ "invalid", test_set_state_invalid, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_invalid },
	{ "shorts", test_set_state_shorts, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_states_shorts },
	NULL
};

const MunitSuite ts_set_state = { "commands/_set_state/", ts_set_state_tests,
				  NULL, 1, MUNIT_SUITE_OPTION_NONE };
