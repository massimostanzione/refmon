#ifndef REFMON_SAFETY_H
#define REFMON_SAFETY_H

#define REFMON_SAFETY_CHECK_NONE 0x0
#define REFMON_SAFETY_CHECK_ROOT 0x1
#define REFMON_SAFETY_CHECK_RECONF 0x2
#define REFMON_SAFETY_CHECK_PASSWORD 0x4

int safety_checks(int checks, char *arg);
char *sanitize_user_input_alloc(char __user *unsafe_input);

#endif //REFMON_SAFETY_H
