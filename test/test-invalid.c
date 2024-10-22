#include <errno.h>
#include <unistd.h>
#include "test.h"
#include "munit/munit.h"
#include "api/testapi.h"

static MunitParameterEnum params_invalid_with_states[] = {
	{ "sudo", strings_sudo },
	{ "state", strings_states_valid },
	{ "command", strings_invalid },
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
	munit_assert_int(ret, ==, -EINVAL);

	return MUNIT_OK;
}

static MunitTest ts_invalid_tests[] = { { "all", test_simples, NULL, NULL,
					  MUNIT_TEST_OPTION_NONE,
					  params_invalid_with_states },
					NULL };

const MunitSuite ts_invalid = { "commands/invalid/", ts_invalid_tests, NULL, 1,
				MUNIT_SUITE_OPTION_NONE };
