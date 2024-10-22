#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "../test.h"
#include "../munit/munit.h"
#include "testapi.h"

int test_exec(const char *preamble, const char *cmd, const char *params)
{
	char command[TEST_BSIZE];
	snprintf(command, sizeof(command), "%s %s %s > /dev/null", preamble,
		 cmd, params);
	int ret = (system(command));

	if (ret != 0) {
		int exit_code = WEXITSTATUS(ret);
		if (exit_code >= 256) {
			int real_error = -256 - exit_code;
			return real_error;
		} else {
			return -(256 - exit_code);
		}
	}
	return 0;
}

int test_set_state(const char *state)
{
	// the command
	int ret = test_exec("sudo ", TEST_PREAMBLE_SET_STATE, state);
	munit_assert_int(ret, ==, 0);

	// now check back
	char *out = malloc(MALLOC_STATE * sizeof(char));
	ret = test_exec_output("", TEST_COMMAND_PRINT_STATE, "", out);
	munit_assert_int(ret, ==, 0);
	int occ = test_string_occurrences(out, state);
	munit_assert_int(occ, ==, 1);
	free(out);

	return ret;
}

int test_add(const char *preamble, const char *path)
{
	return test_exec(preamble, TEST_PREAMBLE_ADD, path);
}

int test_rm(const char *preamble, const char *path)
{
	return test_exec(preamble, TEST_PREAMBLE_RM, path);
}

int test_print_reg(const char *preamble, char *output)
{
	return test_exec_output(preamble, TEST_PREAMBLE_PRINT_REGISTRY, "",
				output);
}

int test_flush(const char *preamble)
{
	return test_exec(preamble, TEST_PREAMBLE_FLUSH, "");
}

int test_is_reconf(const char *state)
{
	return test_string_occurrences(state, "REC-");
}

int test_is_off(const char *state)
{
	return MATCH_STRING(state, "OFF") || MATCH_STRING(state, "REC-OFF");
}

int test_string_occurrences(const char *haystack, const char *needle)
{
	if (needle == NULL || *needle == '\0')
		return 0;
	int count = 0;
	const char *tmp = haystack;
	size_t needle_len = strlen(needle);

	while ((tmp = strstr(tmp, needle))) {
		count++;
		tmp += needle_len;
	}
	return count;
}

int test_exec_output(const char *preamble, const char *cmd, const char *params,
		     char *output)
{
	char command[TEST_BSIZE];
	snprintf(command, sizeof(command), "%s %s %s", preamble, cmd, params);

	FILE *fp;
	if ((fp = popen(command, "r")) == NULL) {
		printf("Error while popening file\n");
		return -1;
	}

	output[0] = '\0';

	char row[TEST_BSIZE];
	size_t current_length = 0;

	// riga per riga
	while (fgets(row, sizeof(row), fp) != NULL) {
		size_t row_length = strlen(row);
		if (current_length + row_length < TEST_BSIZE - 1) {
			memcpy(output + current_length, row, row_length);
			current_length += row_length;
			output[current_length] = '\0';
		} else {
			printf("Buffer overflow\n");
			break;
		}
	}

	if (pclose(fp)) {
		printf("Error while executing command (command='%s', errno='%d')\n",
		       command, errno);
		return -1;
	}

	if (current_length > 0 && output[current_length - 1] == '\n') {
		output[current_length - 1] = '\0';
	}
	return 0;
}

int test_check_path_presence_reg(const char *path)
{
	int ret = -1;
	char out[TEST_BSIZE];
	ret = test_exec_output("", TEST_COMMAND_PRINT_REGISTRY, "", out);
	munit_assert_int(ret, ==, 0);
	ret = test_string_occurrences(out, path);
	return ret;
}

int test_count_file_lines(const char *filename, char file_content[])
{
	FILE *file = fopen(filename, "r");
	if (!file)
		return -1;

	char buf[TEST_BSIZE];
	int counter = 0;
	size_t total_size = 0;

	for (;;) {
		size_t res = fread(buf, 1, TEST_BSIZE, file);
		if (ferror(file)) {
			fclose(file);
			return -1;
		}

		for (size_t i = 0; i < res; i++) {
			if (buf[i] == '\n')
				counter++;
		}

		if (total_size + res >= TEST_BSIZE) {
			fclose(file);
			return -1;
		}

		memcpy(file_content + total_size, buf, res);
		total_size += res;

		if (feof(file))
			break;
	}

	if (total_size > 0 && file_content[total_size - 1] == '\n') {
		total_size--;
	}

	file_content[total_size] = '\0';
	fclose(file);
	return counter;
}

int test_count_str_lines(const char *str)
{
	int count = 0;
	const char *ptr = str;

	while (*ptr) {
		if (*ptr == '\n') {
			count++;
		}
		ptr++;
	}

	if (count > 0 || (str[0] != '\0' && str[strlen(str) - 1] != '\n')) {
		count++;
	}

	return count;
}