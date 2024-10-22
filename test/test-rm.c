#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

#define TEST_RM_FILES_NO 10

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
	for (size_t i = 0; i < TEST_RM_FILES_NO; i++) {
		ret = test_add("sudo ", strings_paths_valid[i]);
		munit_assert_int(ret, ==, 0);
	}
	return 0;
}

static MunitResult test_rm_valid(const MunitParameter params[], void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);
	ret = test_rm(sudo, path);
	presences = test_check_path_presence_reg(path);
	if (!TEST_IS_SUDO) {
		munit_assert_int(ret, ==, -EACCES);
		munit_assert_int(presences, >, 0);

	} else if (!test_is_reconf(state)) {
		munit_assert_int(ret, ==, -EPERM);

		munit_assert_int(presences, >, 0);
	} else {
		munit_assert_int(ret, ==, 0);
	}

	return MUNIT_OK;
}

static MunitResult test_rm_duplicates(const MunitParameter params[],
				      void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);

	for (int i = 1; i <= DUPLICATE_ITERATIONS; i++) {
		ret = test_rm(sudo, path);
		presences = test_check_path_presence_reg(path);
		if (!TEST_IS_SUDO) {
			munit_assert_int(ret, ==, -EACCES);
			munit_assert_int(presences, >, 0);
		} else if (!test_is_reconf(state)) {
			munit_assert_int(ret, ==, -EPERM);
			munit_assert_int(presences, >, 0);
		} else {
			munit_assert_int(ret, ==, i == 1 ? 0 : -ENOENT);
		}
	}
	return MUNIT_OK;
}

static MunitResult test_rm_invalid(const MunitParameter params[], void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);

	ret = test_exec(sudo, TEST_PREAMBLE_RM, path);
	presences = test_check_path_presence_reg(path);
	int multi_param = test_string_occurrences(path, " ") > 0;
	if (multi_param || MATCH_STRING(path, "")) {
		//all the cases that cannot even passed (argc check)
		munit_assert_int(ret, ==, -EINVAL);
		munit_assert_int(presences, ==, 0);
	} else if (!TEST_IS_SUDO) {
		munit_assert_int(ret, ==, -EACCES);
		munit_assert_int(presences, ==, 0);
	} else if (!test_is_reconf(state)) {
		munit_assert_int(ret, ==, -EPERM);
		munit_assert_int(presences, ==, 0);
	} else {
		munit_assert_int(ret, ==, -ENOENT);
		munit_assert_int(presences, ==, 0);
	} //}

	return MUNIT_OK;
}

static MunitTest ts_rm_tests[] = {
	{ "valid", test_rm_valid, setup, (void*)teardown, MUNIT_TEST_OPTION_NONE,
	  params_paths_valid },
	{ "duplicates", test_rm_duplicates, setup, (void*)teardown,
	  MUNIT_TEST_OPTION_NONE, params_paths_valid },
	{ "invalid", test_rm_invalid, setup, (void*)teardown, MUNIT_TEST_OPTION_NONE,
	  params_paths_invalid },
	NULL
};

const MunitSuite ts_rm = { "commands/rm/", ts_rm_tests, NULL, 1,
			   MUNIT_SUITE_OPTION_NONE };
