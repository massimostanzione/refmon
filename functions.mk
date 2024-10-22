# some checks for refmon makefiles

IS_FS_MOUNTED = if mount | grep -q "on $(1)"; then true; else false; fi
PATH_EXISTS = if [ -e $(1) ]; then true; else false; fi
IS_MODULE_LOADED = if lsmod | grep -q "^$(1)"; then true; else false; fi

