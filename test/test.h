#ifndef REFMON_TEST_H
#define REFMON_TEST_H

#include <stdio.h>
#include "munit/munit.h"

#define TEST_PREAMBLE "refmon "
#define TEST_PREAMBLE_SET_STATE "refmon --set-state "
#define TEST_PREAMBLE_ADD "refmon -a "
#define TEST_PREAMBLE_RM "refmon -r "
#define TEST_PREAMBLE_PRINT_REGISTRY "refmon --print-registry"
#define TEST_PREAMBLE_FLUSH "refmon -x"
#define TEST_COMMAND_PRINT_STATE "refmon --print-state"
#define TEST_COMMAND_PRINT_REGISTRY "refmon --print-registry"

#define MATCH_STRING(str1, str2)                     \
	((strncmp(str1, str2, strlen(str1)) == 0) && \
	 (strlen(str1) == strlen(str2)))

#define TESTBANNER_OFFSET 386
#define TESTBANNER_OFFSET_NET TESTBANNER_OFFSET - 2
#define DUPLICATE_ITERATIONS 5
#define MALLOC_STATE 516

const MunitSuite ts_steady;
const MunitSuite ts_set_state;
const MunitSuite ts_add;
const MunitSuite ts_rm;
const MunitSuite ts_simples;
const MunitSuite ts_invalid;
const MunitSuite ts_files;
const MunitSuite ts_operations;
const MunitSuite ts_flush;

static char *strings_sudo[] = { "", "sudo ", NULL };

static char *strings_states_valid[] = { "REC-ON", "ON", "OFF", "REC-OFF",
					NULL };

static char *strings_invalid[] = {
	"--foo", "foo", "--refmon", "two args", "one two three", "", NULL, NULL
};

static MunitParameterEnum params_states_valid[] = {
	{ "sudo", strings_sudo },
	{ "state", strings_states_valid },
	{ NULL, NULL },
};
static MunitParameterEnum params_invalid[] = {
	{ "sudo", strings_sudo },
	{ "invalid", strings_invalid },
	{ NULL, NULL },
};

static char *strings_paths_duplicate[] = { "0", "1", NULL };

static char *strings_paths_valid[] = { "testenv/file.txt",
				       "testenv/hard_link",
				       "testenv/symlink",
				       "testenv/hl_hl",
				       "testenv/hl_sl",
				       "testenv/sl_hl",
				       "testenv/sl_sl",
				       "testenv/subfolder",
				       "testenv",
				       "testenv/subfolder/other_file.txt",
				       NULL };

static MunitParameterEnum params_paths_valid[] = {
	{ "state", strings_states_valid },
	{ "sudo", strings_sudo },
	{ "path", strings_paths_valid },
	{ NULL, NULL },
};

static MunitParameterEnum params_paths_invalid[] = {
	{ "state", strings_states_valid },
	{ "sudo", strings_sudo },
	{ "path", strings_invalid },
	{ NULL, NULL },
};

#endif //REFMON_TEST_H
