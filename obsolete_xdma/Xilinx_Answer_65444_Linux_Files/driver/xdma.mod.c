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

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x59253af, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0x1429df6b, __VMLINUX_SYMBOL_STR(device_remove_file) },
	{ 0xb6fcec34, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x2a098b39, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x4fcf516a, __VMLINUX_SYMBOL_STR(pci_bus_read_config_byte) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xd49c3a37, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xb3df9363, __VMLINUX_SYMBOL_STR(sg_init_table) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
	{ 0xec2ac905, __VMLINUX_SYMBOL_STR(__ll_sc_atomic_sub_return) },
	{ 0xf33847d3, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0xd8e484f0, __VMLINUX_SYMBOL_STR(register_chrdev_region) },
	{ 0xc3782eb8, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xaba8ebc6, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0x84bc974b, __VMLINUX_SYMBOL_STR(__arch_copy_from_user) },
	{ 0x8efb1ee3, __VMLINUX_SYMBOL_STR(__dynamic_pr_debug) },
	{ 0x631e6e03, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x8275f775, __VMLINUX_SYMBOL_STR(kobject_set_name) },
	{ 0x76d60060, __VMLINUX_SYMBOL_STR(put_zone_device_page) },
	{ 0x3c42cb97, __VMLINUX_SYMBOL_STR(sg_next) },
	{ 0xd8688729, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0xeaa6270b, __VMLINUX_SYMBOL_STR(pcie_capability_clear_and_set_word) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0xa87cf413, __VMLINUX_SYMBOL_STR(clear_bit) },
	{ 0x7a2af7b4, __VMLINUX_SYMBOL_STR(cpu_number) },
	{ 0x526c3a6c, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xab40cca9, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0x8510ce42, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xdcb764ad, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xa90752cb, __VMLINUX_SYMBOL_STR(pci_enable_msix) },
	{ 0x97fdbab9, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xfadf2436, __VMLINUX_SYMBOL_STR(memstart_addr) },
	{ 0xdfc05c53, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x43b0c9c3, __VMLINUX_SYMBOL_STR(preempt_schedule) },
	{ 0x622598b1, __VMLINUX_SYMBOL_STR(init_wait_entry) },
	{ 0xe18d80d9, __VMLINUX_SYMBOL_STR(pci_find_capability) },
	{ 0x7f0bf186, __VMLINUX_SYMBOL_STR(device_create_file) },
	{ 0x7856fdb0, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xb35dea8f, __VMLINUX_SYMBOL_STR(__arch_copy_to_user) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xceb0f5bd, __VMLINUX_SYMBOL_STR(cpu_hwcaps) },
	{ 0xa0b04675, __VMLINUX_SYMBOL_STR(vmalloc_32) },
	{ 0xa9be000e, __VMLINUX_SYMBOL_STR(pci_enable_msi_range) },
	{ 0x35b3df96, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x81f3f299, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x5cd885d5, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x96220280, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x65345022, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0xcb128141, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x3382ede, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x31d1f902, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x181f9bf2, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0xae8c4d0c, __VMLINUX_SYMBOL_STR(set_bit) },
	{ 0x2e3cb253, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x5702c523, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x9c5bc552, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x89c2d3e5, __VMLINUX_SYMBOL_STR(dump_page) },
	{ 0x2e0d2f7f, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xbf016a31, __VMLINUX_SYMBOL_STR(pci_iomap) },
	{ 0x5cd2d5bb, __VMLINUX_SYMBOL_STR(vmalloc_to_page) },
	{ 0xc920433a, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x28131c43, __VMLINUX_SYMBOL_STR(dma_release_from_coherent_attr) },
	{ 0x48c1af0a, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0xb2ce072e, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x9b63897d, __VMLINUX_SYMBOL_STR(dma_alloc_from_coherent_attr) },
	{ 0x7fbbe3b1, __VMLINUX_SYMBOL_STR(flush_dcache_page) },
	{ 0x328689d4, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x88db9f48, __VMLINUX_SYMBOL_STR(__check_object_size) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xb05a41d6, __VMLINUX_SYMBOL_STR(__put_page) },
	{ 0x1e0153a7, __VMLINUX_SYMBOL_STR(get_user_pages_fast) },
	{ 0xbd3a7966, __VMLINUX_SYMBOL_STR(pcie_capability_read_word) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00009011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd0000901Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd0000902Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd0000903Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007038sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "9822551CABE60D498B39A45");
