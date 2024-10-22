include $(PWD)/functions.mk

CONFIG_MODULE_SIG=n
CONFIG_DEBUG_INFO=y

DEBUG_CFLAGS += -g -DDEBUG
ccflags-y += ${DEBUG_CFLAGS}
CC += ${DEBUG_CFLAGS}

.PHONY: test

obj-m += mod_refmon.o
mod_refmon-y += refmon.o
mod_refmon-y += lib/scth/scth.o
mod_refmon-y += io/logio.o io/procio.o
mod_refmon-y += misc/misc.o misc/fsutils.o
mod_refmon-y += model/instance.o model/state.o
mod_refmon-y += probing/probing.o probing/probing-handlers.o
mod_refmon-y += syscalls/safety.o syscalls/syscalls.o syscalls/syscalls-handlers.o

all: clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

debug: clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	EXTRA_CFLAGS="$(DEBUG_CFLAGS)"
	

load-fs:
	make -C io/refmonfs clean all
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/io/refmonfs modules FUNCTIONS_MK=$(PWD)/functions.mk
	sudo make -C io/refmonfs mount-fs

load-user:
	make -C user all install

load-usctm:
	cd Linux-sys_call_table-discoverer && make && ./load.sh

mount:
	@if ! $(call IS_MODULE_LOADED,mod_refmon); then \
		sudo insmod mod_refmon.ko  the_syscall_table=$(shell sudo cat /sys/module/the_usctm/parameters/sys_call_table_address); \
	fi

unmount:
	@if $(call IS_MODULE_LOADED,mod_refmon); then \
		sudo rmmod mod_refmon; \
	fi

unload-fs:
	make -C $(PWD)/io/refmonfs clean

unload-usctm:
	@if $(call IS_MODULE_LOADED,the_usctm); then \
		cd Linux-sys_call_table-discoverer && ./unload.sh && make clean; \
	fi

unload-user:
	make -C user clean

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

reload: unmount all mount

run: clean load-fs load-usctm load-user all mount

run-debug: clean load-fs load-usctm load-user debug mount

teardown: unmount clean unload-user unload-usctm unload-fs

test-reload:
	make -C user clean test install

test: test-reload
	make -C ./test all

run-test: run test
