#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x78921aca, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6a5b5422, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0x3eedcbb0, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xe296f5a6, __VMLINUX_SYMBOL_STR(kthread_bind) },
	{ 0x79aa04a2, __VMLINUX_SYMBOL_STR(get_random_bytes) },
	{ 0x67b7090d, __VMLINUX_SYMBOL_STR(alloc_pages_current) },
	{ 0xeb3bce7d, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x3a5a5c3, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xe5f094b1, __VMLINUX_SYMBOL_STR(__free_pages) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x38722f80, __VMLINUX_SYMBOL_STR(kernel_fpu_end) },
	{ 0xd986dad1, __VMLINUX_SYMBOL_STR(kernel_fpu_begin) },
	{ 0xa202a8e5, __VMLINUX_SYMBOL_STR(kmalloc_order_trace) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xc87c1f84, __VMLINUX_SYMBOL_STR(ktime_get) },
	{ 0xc80900a8, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "279AD763C9364680671C673");
