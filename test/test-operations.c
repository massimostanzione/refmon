#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

#define TEST_OPERATIONS_FILES_NO 9

static char *strings_paths_files[] = { "testenv/file.txt",
				       "testenv/hard_link",
				       "testenv/symlink",
				       "testenv/hl_hl",
				       "testenv/hl_sl",
				       "testenv/sl_hl",
				       "testenv/sl_sl",
				       "testenv/subfolder/other_file.txt",
				       NULL };

static char *strings_paths_dirs[] = { "testenv", NULL };

static MunitParameterEnum params_paths_files[] = {
	{ "state", strings_states_valid },
	{ "sudo", strings_sudo },
	{ "file", strings_paths_files },
	{ NULL, NULL },
};

static MunitParameterEnum params_paths_dirs[] = {
	{ "state", strings_states_valid },
	{ "sudo", strings_sudo },
	{ "dir", strings_paths_dirs },
	//{"file",  strings_paths_files},
	{ NULL, NULL },
};

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

void reset()
{
	test_set_state("OFF");
	teardown(NULL);
	test_set_state("OFF");
	setup(NULL, NULL);
}

int write_to_file_test(const char *file)
{
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		return -errno;
	}
	fprintf(fp, "TESTED");

	fclose(fp);
	return 0;
}

static MunitResult test_operations_files(const MunitParameter params[],
					 void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *file = munit_parameters_get(params, "file");

	test_set_state("REC-OFF");
	ret = test_add("sudo ", file);
	munit_assert_int(ret, ==, 0);

	test_set_state(state);

	//READ
	ret = test_exec(sudo, "cat ", file);
	munit_assert_int(ret, ==, 0);

	//WRITE
	ret = write_to_file_test(file);
	munit_assert_int(ret, ==, test_is_off(state) ? 0 : -EACCES);

	//REMOVE
	ret = remove(file);
	munit_assert_int(ret, ==, test_is_off(state) ? 0 : -1);
	if (test_is_off(state)) {
		reset();
	} else {
		munit_assert_int(-errno, ==, -EACCES);
	}

	return MUNIT_OK;
}

static MunitResult test_operations_dirs(const MunitParameter params[],
					void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *dir = munit_parameters_get(params, "dir");

	test_set_state("REC-OFF");
	ret = test_add("sudo ", dir);
	munit_assert_int(ret, ==, 0);

	test_set_state(state);

	for (int i = 0; i < TEST_OPERATIONS_FILES_NO - 1; i++) {
		//READ
		ret = test_exec(sudo, "cat ", strings_paths_files[i]);
		munit_assert_int(ret, ==, 0);

		//WRITE
		ret = write_to_file_test(strings_paths_files[i]);
		munit_assert_int(ret, ==, test_is_off(state) ? 0 : -EACCES);

		//REMOVE
		ret = remove(strings_paths_files[i]);
		munit_assert_int(ret, ==, test_is_off(state) ? 0 : -1);
		if (test_is_off(state)) {
			reset();
		} else {
			munit_assert_int(-errno, ==, -EACCES);
		}
	}

	//MKDIR
	ret = mkdir("testenv/test-new-dir",
		    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	munit_assert_int(ret, ==, test_is_off(state) ? 0 : -1);

	if (test_is_off(state)) {
		reset();
	} else {
		munit_assert_int(-errno, ==, -EACCES);
	}

	//RMDIR
	ret = rmdir("testenv/empty-subdir");
	munit_assert_int(ret, ==, test_is_off(state) ? 0 : -1);
	if (test_is_off(state)) {
		reset();
	} else {
		munit_assert_int(-errno, ==, -EACCES);
	}

	return MUNIT_OK;
}

static MunitTest ts_operations_tests[] = {
	{ "files", test_operations_files, setup, teardown,
	  MUNIT_TEST_OPTION_NONE, params_paths_files },
	{ "dir", test_operations_dirs, setup, teardown, MUNIT_TEST_OPTION_NONE,
	  params_paths_dirs },
	NULL
};

const MunitSuite ts_operations = { "operations/", ts_operations_tests, NULL, 1,
				   MUNIT_SUITE_OPTION_NONE };
