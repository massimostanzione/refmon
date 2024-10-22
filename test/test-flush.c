#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

#define TEST_FLUSH_FILES_NO 6

static MunitTestTearDown teardown(void *fixture)
{
	int ret = test_exec("", "./scripts/teardown.sh", "");
	munit_assert_int(ret, ==, 0);
	return NULL;
}

static void *setup(const MunitParameter params[], void *user_data)
{
	int ret = test_exec("", "./scripts/setup.sh", "");
	munit_assert_int(ret, ==, 0);

	// add all the files to the list (tested elsewhere)
	test_set_state("REC-OFF");
	for (size_t i = 0; i < TEST_FLUSH_FILES_NO; i++) {
		ret = test_add("sudo ", strings_paths_valid[i]);
		munit_assert_int(ret, ==, 0);
	}
	return 0;
}

static MunitResult test_flush_valid(const MunitParameter params[],
				    void *fixture)
{
	int retflush = -1, retout = -1, outlen = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");

	char out[TEST_BSIZE];

	test_set_state(state);

	munit_log(MUNIT_LOG_WARNING, state);
	retflush = test_flush(sudo);
	retout = test_print_reg("", out);
	outlen = strlen(out);
	if (!TEST_IS_SUDO) {
		munit_assert_int(retflush, ==, -EACCES);
	} else if (!test_is_reconf(state)) {
		munit_assert_int(retflush, ==, -EPERM);
	} else {
		munit_assert_int(retflush, ==, 0);
		munit_assert_int(outlen, ==, TESTBANNER_OFFSET_NET);
	}

	return MUNIT_OK;
}

static MunitResult test_flush_invalid(const MunitParameter params[],
				      void *fixture)
{
	int ret = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *extra = munit_parameters_get(params, "path");

	test_set_state(state);
	ret = test_exec(sudo, " refmon --flush ", extra);

	if (!TEST_IS_SUDO) {
		munit_assert_int(ret, ==, -EACCES);
	} else if (!test_is_reconf(state)) {
		munit_assert_int(ret, ==, -EPERM);
	} else {
		munit_assert_int(ret, ==, 0);
	}
	return MUNIT_OK;
}
static MunitTest ts_flush_tests[] = {
	{ "valid", test_flush_valid, setup, (void*)teardown, MUNIT_TEST_OPTION_NONE,
	  params_states_valid },
	{ "invalid", test_flush_invalid, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_paths_invalid },
	NULL
};

const MunitSuite ts_flush = { "commands/flush/", ts_flush_tests, NULL, 1,
			      MUNIT_SUITE_OPTION_NONE };
