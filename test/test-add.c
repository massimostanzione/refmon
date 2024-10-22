#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

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
	return 0;
}

static MunitResult test_add_valid(const MunitParameter params[], void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);

	ret = test_add(sudo, path);
	presences = test_check_path_presence_reg(path);
	if (!TEST_IS_SUDO) {
		munit_assert_int(ret, ==, -EACCES);
		munit_assert_int(presences, ==, 0);
	} else if (!test_is_reconf(state)) {
		munit_assert_int(ret, ==, -EPERM);
		munit_assert_int(presences, ==, 0);
	} else {
		munit_assert_int(ret, ==, 0);
		munit_assert_int(presences, >=, 1);
	}
	return MUNIT_OK;
}

static MunitResult test_add_duplicates(const MunitParameter params[],
				       void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);

	for (int i = 1; i <= DUPLICATE_ITERATIONS; i++) {
		ret = test_add(sudo, path);
		presences = test_check_path_presence_reg(path);
		if (!TEST_IS_SUDO) {
			munit_assert_int(ret, ==, -EACCES);
			munit_assert_int(presences, ==, 0);
		} else if (!test_is_reconf(state)) {
			munit_assert_int(ret, ==, -EPERM);
			munit_assert_int(presences, ==, 0);
		} else {
			munit_assert_int(ret, ==, i == 1 ? 0 : -EEXIST);
			munit_assert_int(presences, >=, 1);
		}
	}
	return MUNIT_OK;
}

static MunitResult test_add_invalid(const MunitParameter params[],
				    void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *path = munit_parameters_get(params, "path");

	test_set_state(state);

	ret = test_add(sudo, path);
	presences = test_check_path_presence_reg(path);

	int multi_param = test_string_occurrences(path, " ") > 0;

	// check that invalid params are not inserted into the program
	if (multi_param || MATCH_STRING(path, "")) {
		munit_assert_int(ret, ==, -EINVAL);
		munit_assert_int(presences, ==, 0);
	} else if (!TEST_IS_SUDO) {
		munit_assert_int(ret, ==, -EACCES);
		munit_assert_int(presences, ==, 0);
	} else if (!test_is_reconf(state)) {
		munit_assert_int(ret, ==, -EPERM);
		munit_assert_int(presences, ==, 0);
	} else {
		munit_assert_int(ret, ==, -EINVAL);
		munit_assert_int(presences, ==, 0);
	}
	return MUNIT_OK;
}

static MunitTest ts_add_tests[] = {
	{ "valid", test_add_valid, setup,  (void*)teardown, MUNIT_TEST_OPTION_NONE,
	  params_paths_valid },
	{ "duplicates", test_add_duplicates, setup,  (void*)teardown,
	  MUNIT_TEST_OPTION_NONE, params_paths_valid },
	{ "invalid", test_add_invalid, setup,  (void*)teardown, MUNIT_TEST_OPTION_NONE,
	  params_paths_invalid },
	NULL
};

const MunitSuite ts_add = { "commands/add/", ts_add_tests, NULL, 1,
			    MUNIT_SUITE_OPTION_NONE };
