#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

static char *strings_simples[] = { "--print-registry",
				   "--print-state",
				   "--opinfo",
				   "-i",
				   "--help",
				   "-h",
				   NULL };

static MunitParameterEnum params_simples[] = {
	{ "sudo", strings_sudo },
	{ "state", strings_states_valid },
	{ "command", strings_simples },
	{ NULL, NULL },
};

static MunitParameterEnum params_simples_invalid[] = {
	{ "sudo", strings_sudo },
	{ "state", strings_states_valid },
	{ "command", strings_simples },
	{ "extra", strings_invalid },
	{ NULL, NULL },
};

static MunitResult test_simples(const MunitParameter params[], void *fixture)
{
	int ret = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *command = munit_parameters_get(params, "command");

	test_set_state(state);

	ret = test_exec(sudo, TEST_PREAMBLE, command);
	munit_assert_int(ret, ==, 0);

	return MUNIT_OK;
}

static MunitResult test_simples_invalid(const MunitParameter params[],
					void *fixture)
{
	int ret = -1, presences = -1;
	const char *state = munit_parameters_get(params, "state");
	const char *sudo = munit_parameters_get(params, "sudo");
	const char *command = munit_parameters_get(params, "command");
	const char *extra = munit_parameters_get(params, "path");

	test_set_state(state);

	ret = test_exec(sudo, strcat(TEST_PREAMBLE, command),
			strcat(" ", extra));

	munit_assert_int(ret, ==, strlen(extra) == 0 ? 0 : -EINVAL);
	return MUNIT_OK;
}

static MunitTest ts_simples_tests[] = {

	{ "all", test_simples, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_simples },
	{ "all_invalid", test_simples, NULL, NULL, MUNIT_TEST_OPTION_NONE,
	  params_simples_invalid },
	NULL
};

const MunitSuite ts_simples = { "commands/simple-commands/", ts_simples_tests,
				NULL, 1, MUNIT_SUITE_OPTION_NONE };
