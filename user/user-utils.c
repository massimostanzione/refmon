#include <asm-generic/errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "user.h"

int get_syscall_index(int index)
{
	FILE *file = fopen(PROC_USER_SYSCALL_FILE, "r");
	if (file == NULL) {
		perror("Error while trying to open syscall codes proc file");
		return -1;
	}
	int codes[SC_NO];
	for (int i = 0; i < SC_NO; i++) {
		int ret = fscanf(file, "%d\n", &codes[i]);
		if (ret != 1) {
			perror("Error while reading from syscall codes proc file");
			fclose(file);
			return -EINVAL;
		}
	}
	fclose(file);
	return codes[index];
}

int read_procfile_from_user(char *filepath)
{
	int ret = -1;
	FILE *fp;
	char output[PIPE_BUF];
	char *cmd = calloc(PATH_MAX, sizeof(char));
	if (cmd == NULL) {
		perror("Error while trying to allocate memory\n");
		ret = -ENOMEM;
		goto tail;
	}
	cmd = strcat(cmd, "cat  ");
	cmd = strcat(cmd, filepath);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		perror("Error while reading procfile");
		ret = -EIO;
		goto tail;
	}

	while (fgets(output, sizeof(output), fp) != NULL) {
		printf("%s", output);
	}

	if (pclose(fp) == -1) {
		perror("Error while closing procfile");
		ret = -EIO;
		goto tail;
	}
	ret = 0;
tail:
	if (cmd)
		free(cmd);
	return ret;
}

char *ask_for_password()
{
#ifdef TEST
	// skip password prompt
	return "ciao";
#endif
	char *input = getpass("Insert password: ");
	if (input == NULL) {
		perror("Error while prompting password\n");
		return NULL;
	}
	return input;
}
