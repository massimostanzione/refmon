#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x6e5e9ea5, "module_layout" },
	{ 0xb8ef88a6, "d_path" },
	{ 0x2d3385d3, "system_wq" },
	{ 0x2750f62d, "kernel_write" },
	{ 0xe157ef13, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x2a58ae55, "disable_kprobe" },
	{ 0xd2edf037, "crypto_alloc_shash" },
	{ 0x754d539c, "strlen" },
	{ 0xe90c1f05, "register_kretprobe" },
	{ 0x56470118, "__warn_printk" },
	{ 0x1a1279de, "remove_proc_entry" },
	{ 0x837b7b09, "__dynamic_pr_debug" },
	{ 0x1fbbe13b, "filp_close" },
	{ 0xc740585c, "crypto_shash_final" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xba6c492f, "pv_ops" },
	{ 0x2d39b0a7, "kstrdup" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0xc5de3352, "kernel_read" },
	{ 0x5a61208a, "kern_path" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0xf8aae665, "current_task" },
	{ 0xc5850110, "printk" },
	{ 0x55a0046c, "crypto_shash_update" },
	{ 0x8d617274, "unregister_kretprobe" },
	{ 0x5a921311, "strncmp" },
	{ 0xb309b99c, "dentry_path_raw" },
	{ 0x2ac2dfdf, "__task_pid_nr_ns" },
	{ 0xa916b694, "strnlen" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x789602ef, "crypto_destroy_tfm" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x5e0b5cfb, "path_put" },
	{ 0xf163e0f9, "kmem_cache_alloc_trace" },
	{ 0xdbf17652, "_raw_spin_lock" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x37a0cba, "kfree" },
	{ 0xc85b066d, "enable_kprobe" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xf914135c, "proc_create" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x9060573, "param_ops_ulong" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xa95faa8e, "filp_open" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "2209673701D55851CF5A21A");
