#ifndef REFMON_TESTAPI_H
#define REFMON_TESTAPI_H

#include <stdio.h>
#include "../munit/munit.h"

#define TEST_IS_SUDO (sudo != "")
#define TEST_HACKED_ENTRIES 4

#define TEST_BSIZE BUFSIZ

// interaction
int test_set_state(const char *state);
int test_add(const char *preamble, const char *path);
int test_rm(const char *preamble, const char *path);
int test_print_reg(const char *preamble, char *output);
int test_flush(const char *preamble);

// execution
int test_exec(const char *preamble, const char *cmd, const char *params);
int test_exec_output(const char *preamble, const char *cmd, const char *params,
		     char *output);

//checks
int test_is_reconf(const char *state);
int test_is_off(const char *state);
int test_string_occurrences(const char *haystack, const char *needle);
int test_check_path_presence_reg(const char *path);
int test_count_file_lines(const char *filename, char file_content[]);
int test_count_str_lines(const char *str);

#endif //REFMON_TESTAPI_H
