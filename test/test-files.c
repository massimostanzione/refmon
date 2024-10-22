#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

#define TEST_FILE_SYSCALLCODES "/proc/refmon-syscallcodes"
#define TEST_FILE_REGISTRY "/proc/refmon-list"
#define TEST_FILE_STATE "/proc/refmon-state"

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

	test_set_state("REC-OFF");

	ret = test_exec("sudo ", TEST_PREAMBLE_ADD, "/dev/null");
	munit_assert_int(ret, ==, 0);
	return 0;
}

static MunitResult test_files_proc_list(const MunitParameter params[],
					void *fixture)
{
	int ret = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");

	test_set_state(state);

	char out_cat[TEST_BSIZE];
	char out_C[TEST_BSIZE];
	char out_cmd[TEST_BSIZE];
	char out_opinfo[TEST_BSIZE];

	// 1. controllo il contenuto
	//  1.1 da cat
	ret = test_exec_output(sudo, "cat ", TEST_FILE_REGISTRY, out_cat);
	munit_assert_int(ret, ==, 0);

	//  1.2 da C
	test_count_file_lines(TEST_FILE_REGISTRY, out_C);

	//  1.3 da comando
	ret = test_exec_output(sudo, TEST_PREAMBLE, "--print-registry",
			       out_cmd);
	munit_assert_int(ret, ==, 0);
	munit_assert_string_equal(out_cat, out_C);
	int i = 0, pos = TESTBANNER_OFFSET;
	char substring[TEST_BSIZE];
	strncpy(substring, out_cmd + (pos - 1), strlen(out_cmd));
	munit_assert_string_equal(substring, out_C);

	//2. comando misto opinfo
	ret = test_exec_output(sudo, TEST_PREAMBLE, "--opinfo", out_opinfo);
	munit_assert_int(ret, ==, 0);

	munit_assert_int(test_string_occurrences(out_opinfo, out_cat), ==, 1);

	return MUNIT_OK;
}

static MunitResult test_files_proc_state(const MunitParameter params[],
					 void *fixture)
{
	int ret = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");

	test_set_state(state);

	char out_cat[TEST_BSIZE];
	char out_C[TEST_BSIZE];
	char out_cmd[TEST_BSIZE];
	char out_opinfo[TEST_BSIZE];

	// 1. controllo il contenuto
	//  1.1 da cat
	ret = test_exec_output(sudo, "cat ", TEST_FILE_STATE, out_cat);
	munit_assert_int(ret, ==, 0);

	//  1.2 da C
	test_count_file_lines(TEST_FILE_STATE, out_C);

	//  1.3 da comando
	ret = test_exec_output(sudo, TEST_PREAMBLE, "--print-state", out_cmd);
	munit_assert_int(ret, ==, 0);
	munit_assert_string_equal(out_cat, out_C);
	int i = 0, pos = TESTBANNER_OFFSET;
	char substring[TEST_BSIZE];
	strncpy(substring, out_cmd + (pos - 1), strlen(out_cmd));
	munit_assert_string_equal(substring, out_C);

	//2. comando misto opinfo
	ret = test_exec_output(sudo, TEST_PREAMBLE, "--opinfo", out_opinfo);
	munit_assert_int(ret, ==, 0);

	munit_assert_int(test_string_occurrences(strcat(out_opinfo, "\n"),
						 out_cat),
			 ==, 1);

	return MUNIT_OK;
}

static MunitResult test_files_proc_syscallcodes(const MunitParameter params[],
						void *fixture)
{
	int ret = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");

	test_set_state(state);

	// 1. conto il numero di syscalls
	//  1.1 da cat
	char out[TEST_BSIZE];
	char outC[TEST_BSIZE];
	ret = test_exec_output(sudo, "cat ", TEST_FILE_SYSCALLCODES, out);
	munit_assert_int(ret, ==, 0);
	munit_assert_int(test_count_str_lines(out), ==, TEST_HACKED_ENTRIES);
	//  1.2 da C
	munit_assert_int(test_count_file_lines(TEST_FILE_SYSCALLCODES, outC),
			 ==, TEST_HACKED_ENTRIES);

	// 2. controllo il contenuto
	munit_assert_int(strlen(out), ==, strlen(outC));
	munit_assert_string_equal(out, outC);

	return MUNIT_OK;
}

static MunitTest ts_files_tests[] = {
	{ "proc-list", test_files_proc_list, setup, (void*)teardown,
	  MUNIT_TEST_OPTION_NONE, params_states_valid },
	{ "proc-state", test_files_proc_state, NULL, NULL,
	  MUNIT_TEST_OPTION_NONE, params_states_valid },
	{ "proc-syscallcodes", test_files_proc_syscallcodes, NULL, NULL,
	  MUNIT_TEST_OPTION_NONE, params_states_valid },
	NULL
};

const MunitSuite ts_files = { "files/", ts_files_tests, NULL, 1,
			      MUNIT_SUITE_OPTION_NONE };
