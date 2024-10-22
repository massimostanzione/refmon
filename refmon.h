#ifndef REFMON_REFMON_H
#define REFMON_REFMON_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "model/instance.h"

#define REFMON_MODNAME "refmon"
#define REFMON_RETVAL_DEFAULT (-1)

#define FREE(b)        \
	if (b != NULL) \
	kfree(b)

#endif //REFMON_REFMON_H
