#include <stdio.h>
#include "test.h"
#include "api/testapi.h"

static MunitSuite suites[] = {};

MunitSuite ts_main = { "tests/", NULL, suites, 1, MUNIT_SUITE_OPTION_NONE };

int main(int argc, char **argv)
{
	printf("+++++++++++++++++\n");
	printf("testing\n");
	printf("+++++++++++++++++\n");

	test_set_state("REC-OFF");

	suites[0] = ts_steady;
	suites[1] = ts_set_state;
	suites[2] = ts_add;
	suites[3] = ts_rm;
	suites[4] = ts_simples;
	suites[5] = ts_invalid;
	suites[6] = ts_files;
	suites[7] = ts_operations;
	suites[8] = ts_flush;

	return munit_suite_main(&ts_main, NULL, argc, argv);
}
