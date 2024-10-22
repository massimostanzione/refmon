#ifndef REFMON_MISC_H
#define REFMON_MISC_H

#include "../model/state.h"
#include "../model/registry.h"

#define REFMON_DEFAULT_PASSWORD "ciao"

#define REFMON_CRYPT_ALGO "sha256"

#define MATCH_STRING(str1, str2)                     \
	((strncmp(str1, str2, strlen(str1)) == 0) && \
	 (strlen(str1) == strlen(str2)))

char *hashgen_str(const char *str);
char *hashgen_filecont(const char *filepath);
int check_list_init(struct list_head *list);

#endif //REFMON_MISC_H
