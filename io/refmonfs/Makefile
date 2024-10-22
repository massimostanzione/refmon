FUNCTIONS_MK ?= ../../functions.mk
include $(FUNCTIONS_MK)

MOUNT_PATH = /mnt/refmonfs
MODNAME = refmonfs

obj-m += refmonfs.o
refmonfs-objs += singlefilefs.o file.o dir.o

all:
	gcc singlefilemakefs.c -o singlefilemakefs
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	@if $(call IS_FS_MOUNTED,$(MOUNT_PATH)); then	\
		sudo umount "$(MOUNT_PATH)";				\
	fi
	@if $(call IS_MODULE_LOADED,$(MODNAME)); then	\
		sudo rmmod "$(MODNAME)";					\
	fi
	rm -f image
	rm -f singlefilemakefs
	@if $(call PATH_EXISTS,$(MOUNT_PATH)); then		\
		sudo rmdir "$(MOUNT_PATH)";					\
	fi

mount-fs: create-fs mount
	@if ! $(call PATH_EXISTS,$(MOUNT_PATH)); then	\
        mkdir "$(MOUNT_PATH)";						\
    fi
	mount -o loop -t singlefilefs image "$(MOUNT_PATH)"

mount:	
	@if $(call IS_MODULE_LOADED,$(MODNAME)); then 	\
		sudo rmmod "$(MODNAME)";					\
	fi
	insmod "$(MODNAME)".ko

create-fs:
	dd bs=4096 count=100 if=/dev/zero of=image
	./singlefilemakefs image
