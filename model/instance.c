#include "instance.h"
#include "state.h"

struct refmon the_instance;

int is_off(void)
{
	return the_instance.state == OFF || the_instance.state == REC_OFF;
}

int is_on(void)
{
	return !is_off();
}

int is_reconf(void)
{
	return the_instance.state == REC_OFF || the_instance.state == REC_ON;
}