#include <asm-generic/errno.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include "user.h"

int cmd_set_state(char *new_state)
{
	int syscall_index = get_syscall_index(SC_ORDER_SET_STATE);
	if (syscall_index < 0) {
		printf("Error while trying to get syscall code for _set_state\n");
		return syscall_index;
	}

	int ret = syscall(syscall_index, new_state);
	if (ret != 0) {
		printf("An error occurred while attempting syscall:\n");
	}
	switch (-errno) {
	case 0:
		printf("State changed to %s.\n", new_state);
		return RETURN_OK;
	case -EINVAL:
		printf("Invalid parameter. Please make sure the state specified is either one of the followings: OFF ON REC-OFF REC-ON\n");
		break;
	case -EACCES:
		printf("Access denied. This operation requires you to be root.\n");
		break;
	default:
		perror("Unexpected error");
		break;
	}
	return -errno;
}

int cmd_reconf_add(char *path, char *password)
{
	int syscall_index = get_syscall_index(SC_ORDER_RECONF_ADD);
	if (syscall_index < 0) {
		printf("Error while trying to get syscall code for reconf_add\n");
		return syscall_index;
	}
	int ret = syscall(syscall_index, path, password);
	if (ret != 0) {
		printf("An error occurred while attempting syscall:\n");
	}
	switch (-errno) {
	case 0:
		printf("Path %s ADDED to the reference monitor.\n", path);
		return RETURN_OK;
	case -EINVAL:
		printf("Invalid parameter. Please check again your password and the path you submitted.\n");
		break;
	case -EACCES:
		printf("Access denied. Either you are non-root or the password you prompted is wrong.\n");
		break;
	case -ENOMEM:
		printf("Memory error while executing syscall.\n");
		break;
	case -EEXIST:
		printf("The path is already registered.\n");
		break;
	case -EPERM:
		printf("Operation not permitted. RefMon is in a non-reconfigurable state. Please change the state to either REC-OFF or REC-ON.\n");
		break;
	default:
		perror("Unexpected error");
		break;
	}
	return -errno;
}

int cmd_reconf_rm(char *path, char *password)
{
	int syscall_index = get_syscall_index(SC_ORDER_RECONF_RM);
	if (syscall_index < 0) {
		printf("Error while trying to get syscall code for reconf_rm\n");
		return syscall_index;
	}
	int ret = syscall(syscall_index, path, password);
	if (ret != 0) {
		printf("An error occurred while attempting syscall:\n");
	}
	switch (-errno) {
	case 0:
		printf("Path %s REMOVED from the reference monitor.\n", path);
		return RETURN_OK;
	case -EINVAL:
		printf("Invalid parameter. Please check again your password and the path you submitted.\n");
		break;
	case -EACCES:
		printf("Access denied. Either you are non-root or the password you prompted is wrong.\n");
		break;
	case -ENOMEM:
		printf("Memory error while executing syscall.\n");
		break;
	case -ENOENT:
		printf("The path is not registered.\n");
		break;
	case -EPERM:
		printf("Operation not permitted. RefMon is in a non-reconfigurable state. Please change the state to either REC-OFF or REC-ON.\n");
		break;
	default:
		perror("Unexpected error");
		break;
	}
	return -errno;
}

int cmd_print_state()
{
	int ret = read_procfile_from_user(PROC_USER_STATE_FILE);
	if (ret < 0) {
		perror("Error while reading proc state file");
		return -errno;
	}
	return RETURN_OK;
}

int cmd_print_registry()
{
	int ret = read_procfile_from_user(PROC_USER_LIST_FILE);
	if (ret < 0) {
		perror("Error while reading proc list file");
		return -ret;
	}
	return RETURN_OK;
}

int cmd_opinfo()
{
	int ret = -1;
	printf("░░░░░░░░░░░░░░░░░░░\n"
	       "░░               ░░\n"
	       "░░  R e f M o n  ░░\n"
	       "░░               ░░\n"
	       "░░░░░░░░░░░░░░░░░░░\n"
	       "Info about current operations:\n"
	       "-----------------------------\n"
	       "State: ");
	ret = cmd_print_state();
	if (ret != 0)
		goto tail_err;
	printf("Pathlist:\n");
	ret = cmd_print_registry();
	if (ret != 0)
		goto tail_err;
	return RETURN_OK;
tail_err:
	return ret;
}

int cmd_help()
{
	printf("Usage: refmon OPTIONS [FILE]\n");
	printf("Reference monitor, for enhanced file access protection.\n\n");
	printf("Options:\n");
	printf("  -s, --set-state <STATE>     Set state to STATE\n");
	printf("      --off                   Set state to OFF (alias for --set-state OFF)\n");
	printf("      --on                    Set state to ON (alias for --set-state ON)\n");
	printf("      --recoff                Set state to REC-OFF (alias for --set-state REC-OFF)\n");
	printf("      --recon                 Set state to REC-ON (alias for --set-state REC-ON)\n");
	printf("  -a, --reconf-add <PATH>     Add PATH to refmon registry\n");
	printf("  -r, --reconf-rm <PATH>      Remove PATH from refmon registry\n");
	printf("      --print-state           Print current state\n");
	printf("      --print-registry        Print the registry\n");
	printf("  -i, --opinfo                Print operational info (more human-friendly)\n");
	printf("  -h, --help                  Show this help message\n");
#ifdef TEST
	printf("  -x, --flush                 Flush the refmon registry (test only)\n");
#endif
	return RETURN_OK;
}

int cmd_flush(char *password)
{
	int syscall_index = get_syscall_index(SC_ORDER_FLUSH);
	if (syscall_index < 0) {
		printf("Error while trying to get syscall code for flush\n");
		return syscall_index;
	}
	int ret = syscall(syscall_index, password);
	if (ret != 0) {
		printf("An error occurred while attempting syscall:\n");
	}
	switch (-errno) {
	case 0:
		printf("Registry flushed. [*** REMEMBER: YOU ARE IN TEST MODE ***]\n");
		return RETURN_OK;
	default:
		perror("Unexpected error");
	}
	return -errno;
}

void print_usage(const char *prog_name)
{
	printf("Usage: %s COMMAND [ARGS...]\nRun '%s --help' for further information.\n",
	       prog_name, prog_name);
}

int main(int argc, char *argv[])
{
#ifdef TEST
	printf("******************************************************************\n");
	printf("*** WARNING: you are running Refmon in TEST mode!\n");
	printf("***          password prompt is DISABLED!!!\n");
	printf("***          this may lead to security issues,\n");
	printf("***          please use for test purposes onlu\n");
	printf("***          and remember to re-compile without the TEST flag!\n");
	printf("******************************************************************\n");
#endif
	if (argc < 2) {
		print_usage(argv[0]);
		return -EINVAL;
	}
	int opt;
	int option_index = 0;
	char *password = NULL;

	static struct option long_options[] = {
		{ "set-state", required_argument, 0, 's' },
		{ "off", no_argument, 0, 0 },
		{ "on", no_argument, 0, 0 },
		{ "recoff", no_argument, 0, 0 },
		{ "recon", no_argument, 0, 0 },
		{ "reconf-add", required_argument, 0, 'a' },
		{ "reconf-rm", required_argument, 0, 'r' },
		{ "print-state", no_argument, 0, 0 },
		{ "print-registry", no_argument, 0, 0 },
		{ "opinfo", no_argument, 0, 'i' },
		{ "help", no_argument, 0, 'h' },
#ifdef TEST
		{ "flush", no_argument, 0, 'x' },
#endif
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "s:a:r:ihx", long_options,
				  &option_index)) != -1) {
		switch (opt) {
		case 's':
			if (argc != 3) {
				printf("Wrong arguments number. See --help flag for details.\n");
				return -EINVAL;
			}
			return cmd_set_state(optarg);
		case 'a':
			if (argc != 3) {
				printf("Wrong arguments number. See --help flag for details.\n");
				return -EINVAL;
			}
			password = ask_for_password();
			if (password == NULL) {
				perror("Error while prompting password\n");
				return -EINVAL;
			}
			return cmd_reconf_add(optarg, password);
		case 'r':
			if (argc != 3) {
				printf("Wrong arguments number. See --help flag for details.\n");
				return -EINVAL;
			}
			password = ask_for_password();
			if (password == NULL) {
				perror("Error while prompting password\n");
				return -EINVAL;
			}
			return cmd_reconf_rm(optarg, password);
		case 'i':
			if (argc > 2) {
				printf("Ignoring extra arguments\n");
			}
			return cmd_opinfo();
		case 'h':
			if (argc > 2) {
				printf("Ignoring extra arguments\n");
			}
			return cmd_help();
		case 0:
			MATCH_COMMAND("off") return cmd_set_state("OFF");
			MATCH_COMMAND("on") return cmd_set_state("ON");
			MATCH_COMMAND("recoff") return cmd_set_state("REC-OFF");
			MATCH_COMMAND("recon") return cmd_set_state("REC-ON");
			MATCH_COMMAND("print-state") return cmd_print_state();
			MATCH_COMMAND("print-registry")
			return cmd_print_registry();
			break;
#ifdef TEST
		case 'x':
			password = ask_for_password();
			if (password == NULL) {
				perror("Error while prompting password\n");
				return -EINVAL;
			}
			return cmd_flush(password);
#endif
		default:
			print_usage(argv[0]);
			return -EINVAL;
		}
	}
	print_usage(argv[0]);
	return -EINVAL;
}
