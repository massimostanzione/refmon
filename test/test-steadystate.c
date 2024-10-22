#include <stdio.h>
#include <unistd.h>
#include "munit/munit.h"
#include "api/testapi.h"

static char *strings_modules[] = { "mod_refmon", "refmonfs", "the_usctm",
				   NULL };

static char *strings_files[] = { "/usr/local/bin/refmon",
				 "/proc/refmon-list",
				 "/proc/refmon-state",
				 "/proc/refmon-syscallcodes",
				 "/mnt/refmonfs/blocked-ops.log",
				 NULL };

static MunitParameterEnum params_modules[] = {
	{ "module", strings_modules },
	{ NULL, NULL },
};

static MunitParameterEnum params_files[] = {
	{ "file", strings_files },
	{ NULL, NULL },
};

static MunitResult test_steady_modules(const MunitParameter params[],
				       void *fixture)
{
	const char *module = munit_parameters_get(params, "module");
	char out[TEST_BSIZE];
	int ret = test_exec_output("", "lsmod | grep ", module, out);
	munit_assert_int(ret, ==, 0);
	munit_assert_int(strlen(out), >, 0);
	return MUNIT_OK;
}

static MunitResult test_steady_files(const MunitParameter params[],
				     void *fixture)
{
	const char *file = munit_parameters_get(params, "file");
	int ret = test_exec("", "test -f ", file);
	munit_assert_int(ret, ==, 0);
	return MUNIT_OK;
}

static MunitTest ts_steady_tests[] = { { "modules", test_steady_modules, NULL,
					 NULL, MUNIT_TEST_OPTION_NONE,
					 params_modules },
				       { "files", test_steady_files, NULL, NULL,
					 MUNIT_TEST_OPTION_NONE, params_files },
				       NULL };

const MunitSuite ts_steady = { "steady-state/", ts_steady_tests, NULL, 1,
			       MUNIT_SUITE_OPTION_NONE };
