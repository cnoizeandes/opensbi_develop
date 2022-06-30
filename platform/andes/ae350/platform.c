/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_io.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_trap.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include <libfdt.h>
#include "platform.h"
#include "plicsw.h"
#include "plmt.h"
#include "cache.h"
#include "trigger.h"
#include "smu.h"
#include "wdt.h"
#include "pma.h"

static struct plic_data plic = {
	.addr = AE350_PLIC_ADDR,
	.num_src = AE350_PLIC_NUM_SOURCES,
};
static struct smu_data smu;
static struct wdt_data wdt;
struct l2c_data l2c;

extern struct sbi_platform platform;

unsigned long fw_platform_init(unsigned long arg0, unsigned long arg1,
				unsigned long arg2, unsigned long arg3,
				unsigned long arg4)
{
	void *fdt = (void *)arg1;
	u32 max_hartid;
	int root_offset, cpus_offset;

	root_offset = fdt_path_offset(fdt, "/");
	if (root_offset < 0)
		goto fail;

	/* Get hart count */
	cpus_offset = fdt_parse_max_hart_id(fdt, &max_hartid);
	if (cpus_offset < 0)
		goto fail;

	platform.hart_count = max_hartid + 1;

	// Return original FDT pointer
	return arg1;

fail:
	while (1)
		wfi();
}

static void mcall_set_reset_vec(uint64_t addr)
{
	int i;
	uint32_t cpu_nums;

	cpu_nums = sbi_platform_thishart_ptr()->hart_count;

	uint32_t addr_lo = (uint32_t)addr;
	uint32_t addr_hi = (uint32_t)(addr >> 32);
	for (i = 0; i < cpu_nums; i++) {
		writel(addr_lo, (void *)smu.addr +  SMU_HARTn_RESET_VEC_LO(i));
		writel(addr_hi, (void *)smu.addr +  SMU_HARTn_RESET_VEC_HI(i));
	}
}

static void __noreturn mcall_restart(void)
{
	mcall_set_reset_vec(FLASH_BASE);

	writew(ATCWDT200_WP_NUM, (void *)(wdt.addr + WREN_OFF));
	writel(INT_CLK_32768 | INT_EN | RST_CLK_128 | RST_EN | WDT_EN,
			(void *)(wdt.addr + CTRL_OFF));

	sbi_hart_hang();
	__builtin_unreachable();
}

static int ae350_system_reset_check(u32 type, u32 reason)
{
	switch (type) {
	case SBI_SRST_RESET_TYPE_SHUTDOWN:
		return 1;
	case SBI_SRST_RESET_TYPE_COLD_REBOOT:
		return 255;
	case SBI_SRST_RESET_TYPE_WARM_REBOOT:
	default:
		return 0; /* Unsupported */
	}
}

static void ae350_system_reset(u32 type, u32 reason)
{
	switch (type) {
	case SBI_SRST_RESET_TYPE_SHUTDOWN:
		sbi_hart_hang();
	case SBI_SRST_RESET_TYPE_COLD_REBOOT:
		mcall_restart();
	case SBI_SRST_RESET_TYPE_WARM_REBOOT:
	default:
		sbi_printf("Unsupported reset type : %d\n", type);
		sbi_hart_hang();
	}
}

static struct sbi_system_reset_device ae350_reset = {
	.name = "ae350_wdt",
	.system_reset_check = ae350_system_reset_check,
	.system_reset = ae350_system_reset
};

static int ae350_system_reset_devices_init(void){
	void *fdt;

	fdt = fdt_get_address();

	/* Rebooting requires smu to set the reset vector for each hart */
	if(fdt_parse_compat_addr(fdt, (uint64_t *)&smu.addr, "andestech,atcsmu") ||
			fdt_parse_compat_addr(fdt, (uint64_t *)&wdt.addr, "andestech,atcwdt200"))
		return SBI_ENODEV;

	return 0;
}

static int ae350_hsm_init(void) {
	void *fdt;

	fdt = fdt_get_address();

	/*
	 * No need to parse smu address, ae350_[set|enter]_suspend_mode can't be
	 * used if ATCSMU is not configured in kernel.
	 * If L2C address is 0, cpu suspend/resume will skip L2C disabling/enabling
	 */
	if(fdt_parse_compat_addr(fdt, (uint64_t *)&l2c.addr, "cache"))
		l2c.addr = 0;

	return 0;
}

static int ae350_early_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = ae350_system_reset_devices_init();
		if(!rc)
			sbi_system_reset_add_device(&ae350_reset);
		ae350_hsm_init();
	}

	return 0;
}

/* Platform final initialization. */
static int ae350_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = fdt_get_address();
	fdt_fixups(fdt);

	init_pma();
	trigger_init();

	return 0;
}

static uintptr_t mcall_set_trigger(long type, uintptr_t data, unsigned int m,
					unsigned int s, unsigned int u)
{
	int ret;
	switch (type) {
	case TRIGGER_TYPE_ICOUNT:
		ret = trigger_set_icount(data, m, s, u);
		break;
	case TRIGGER_TYPE_ITRIGGER:
		ret = trigger_set_itrigger(data, m, s, u);
		break;
	case TRIGGER_TYPE_ETRIGGER:
		ret = trigger_set_etrigger(data, m, s, u);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static uintptr_t mcall_set_pfm(void)
{
	csr_clear(CSR_SLIP, MIP_SOVFIP);
	csr_set(CSR_MIE, MIP_MOVFIP);
	return 0;
}

static uintptr_t mcall_suspend_prepare(char main_core, char enable)
{
	smu_suspend_prepare(main_core, enable);
	return 0;
}

extern void cpu_suspend2ram(void);
static uintptr_t mcall_suspend_backup(void)
{
	cpu_suspend2ram();
	return 0;
}

/* Initialize the platform console. */
static int ae350_console_init(void)
{
	return uart8250_init(AE350_UART_ADDR,
			     AE350_UART_FREQUENCY,
			     AE350_UART_BAUDRATE,
			     AE350_UART_REG_SHIFT,
			     AE350_UART_REG_WIDTH);
}

/* Initialize the platform interrupt controller for current HART. */
static int ae350_irqchip_init(bool cold_boot)
{
	u32 hartid = current_hartid();
	int ret;

	if (cold_boot) {
		ret = plic_cold_irqchip_init(&plic);
		if (ret)
			return ret;
	}

	return plic_warm_irqchip_init(&plic, 2 * hartid, 2 * hartid + 1);
}

struct sbi_ipi_device plicsw_ipi = {
	.name = "ae350_plicsw",
	.ipi_send = plicsw_ipi_send,
	.ipi_clear = plicsw_ipi_clear
};

/* Initialize IPI for current HART. */
static int ae350_ipi_init(bool cold_boot)
{
	int ret;
	u32 hart_count = sbi_platform_thishart_ptr()->hart_count;

	if (cold_boot) {
		ret = plicsw_cold_ipi_init(AE350_PLICSW_ADDR,
					   hart_count);
		if (ret)
			return ret;

		sbi_ipi_set_device(&plicsw_ipi);
	}

	return plicsw_warm_ipi_init();
}

/* Initialize platform timer for current HART. */
static int ae350_timer_init(bool cold_boot)
{
	int ret;
	u32 hart_count = sbi_platform_thishart_ptr()->hart_count;

	if (cold_boot) {
		ret = plmt_cold_timer_init(AE350_PLMT_ADDR,
					   hart_count);
		if (ret)
			return ret;
	}

	return plmt_warm_timer_init();
}

/* called flow:
 *
 *	1. kernel -> ae350_set_suspend_mode(light/deep) -> set variable: ae350_suspend_mode
 *
 *	2. cpu_stop() -> sbi_hsm_hart_stop() -> sbi_hsm_exit() ->
 *		jump_warmboot() -> sbi_hsm_hart_wait() -> ae350_enter_suspend_mode() -> normal/light/deep
 */
static int ae350_set_suspend_mode(u32 suspend_mode)
{
	u32 hartid = current_hartid();

	ae350_suspend_mode[hartid] = suspend_mode;
	return 0;
}

int ae350_enter_suspend_mode(bool main_core, unsigned int wake_mask)
{
	u32 hartid, cpu_nums, suspend_mode;

	hartid = current_hartid();
	cpu_nums = sbi_platform_thishart_ptr()->hart_count;

	suspend_mode = ae350_suspend_mode[hartid];

	// smu function
	if (suspend_mode == LightSleepMode) {
		sbi_printf("%s(): CPU[%d] LightSleepMode\n", __func__, hartid);

		// set SMU wakeup enable & MISC control
		smu_set_wakeup_enable(hartid, main_core, wake_mask);
		// Disable higher privilege's non-wakeup event
		smu_suspend_prepare(main_core, false);
		// set SMU light sleep command
		smu_set_sleep(hartid, LightSleep_CTL);
		// Wait for other cores to enter sleeping mode
		if (main_core)
			smu_check_pcs_status(LightSleep_STATUS, cpu_nums);
		// D-cache disable
		mcall_dcache_op(0);
		// wait for interrupt
		wfi();
		// D-cache enable
		mcall_dcache_op(1);
		// enable privilege
		smu_suspend_prepare(main_core, true);
	} else if (suspend_mode == DeepSleepMode) {
		sbi_printf("%s(): CPU[%d] DeepSleepMode\n", __func__, hartid);

		// set SMU wakeup enable & MISC control
		smu_set_wakeup_enable(hartid, main_core, wake_mask);
		// Disable higher privilege's non-wakeup event
		smu_suspend_prepare(main_core, false);
		// set SMU Deep sleep command
		smu_set_sleep(hartid, DeepSleep_CTL);
		// Wait for other cores to enter sleeping mode
		if (main_core)
			smu_check_pcs_status(DeepSleep_STATUS, cpu_nums);
		// stop & wfi & resume
		cpu_suspend2ram();
		// enable privilege
		smu_suspend_prepare(main_core, true);
	} else if (suspend_mode == CpuHotplugDeepSleepMode) {
		/*
		 * In 25-series, core 0 is binding with L2 power domain,
		 * core 0 should NOT enter deep sleep mode.
		 *
		 * For 45-series, every core has its own power domain,
		 * It's ok to sleep main core.
		 */
		if (is_andestar45_series() || !(hartid == 0)) {
			sbi_printf("%s(): CPU[%d] Cpu Hotplug DeepSleepMode\n",
				__func__, hartid);

			// set SMU wakeup enable & MISC control
			smu_set_wakeup_enable(hartid, main_core, 0);
			// Disable higher privilege's non-wakeup event
			smu_suspend_prepare(-1, false);
			// set SMU Deep sleep command
			smu_set_sleep(hartid, DeepSleep_CTL);
			// stop & wfi & resume
			cpu_suspend2ram();
			// enable privilege
			smu_suspend_prepare(-1, true);
		}
	} else {
		sbi_printf("%s(): CPU[%d] Unsupported ae350 suspend mode\n",
				__func__, hartid);
		sbi_hart_hang();
	}

	// reset suspend mode to NormalMode (active)
	ae350_suspend_mode[hartid] = NormalMode;

	return 0;
}

/* Vendor-Specific SBI handler */
static int ae350_vendor_ext_provider(long extid, long funcid,
	const struct sbi_trap_regs *regs, unsigned long *out_value,
	struct sbi_trap_info *out_trap)
{
	int ret = 0;
	switch (funcid) {
	case SBI_EXT_ANDES_GET_MCACHE_CTL_STATUS:
		*out_value = csr_read(CSR_MCACHE_CTL);
		break;
	case SBI_EXT_ANDES_GET_MMISC_CTL_STATUS:
		*out_value = csr_read(CSR_MMISC_CTL);
		break;
	case SBI_EXT_ANDES_SET_MCACHE_CTL:
		ret = mcall_set_mcache_ctl(regs->a0);
		break;
	case SBI_EXT_ANDES_SET_MMISC_CTL:
		ret = mcall_set_mmisc_ctl(regs->a0);
		break;
	case SBI_EXT_ANDES_ICACHE_OP:
		ret = mcall_icache_op(regs->a0);
		break;
	case SBI_EXT_ANDES_DCACHE_OP:
		ret = mcall_dcache_op(regs->a0);
		break;
	case SBI_EXT_ANDES_L1CACHE_I_PREFETCH:
		ret = mcall_l1_cache_i_prefetch_op(regs->a0);
		break;
	case SBI_EXT_ANDES_L1CACHE_D_PREFETCH:
		ret = mcall_l1_cache_d_prefetch_op(regs->a0);
		break;
	case SBI_EXT_ANDES_NON_BLOCKING_LOAD_STORE:
		ret = mcall_non_blocking_load_store(regs->a0);
		break;
	case SBI_EXT_ANDES_WRITE_AROUND:
		ret = mcall_write_around(regs->a0);
		break;
	case SBI_EXT_ANDES_TRIGGER:
		*out_value = mcall_set_trigger(regs->a0, regs->a1, 0, 0, regs->a2);
		break;
	case SBI_EXT_ANDES_SET_PFM:
		ret = mcall_set_pfm();
		break;
	case SBI_EXT_ANDES_READ_POWERBRAKE:
		*out_value = csr_read(CSR_MPFT_CTL);
		break;
	case SBI_EXT_ANDES_WRITE_POWERBRAKE:
		csr_write(CSR_MPFT_CTL, regs->a0);
		break;
	case SBI_EXT_ANDES_SUSPEND_PREPARE:
		ret = mcall_suspend_prepare(regs->a0, regs->a1);
		break;
	case SBI_EXT_ANDES_SUSPEND_MEM:
		ret = mcall_suspend_backup();
		break;
	case SBI_EXT_ANDES_SET_SUSPEND_MODE:
		ae350_set_suspend_mode(regs->a0);
		break;
	case SBI_EXT_ANDES_ENTER_SUSPEND_MODE:
		ae350_enter_suspend_mode(regs->a0, regs->a1);
		break;
	case SBI_EXT_ANDES_RESTART:
		mcall_restart();
		break;
	case SBI_EXT_ANDES_RESET_VEC:
		mcall_set_reset_vec(regs->a0);
		break;
	case SBI_EXT_ANDES_SET_PMA:
		mcall_set_pma(regs->a0, regs->a1, regs->a2, regs->a3);
		break;
	case SBI_EXT_ANDES_FREE_PMA:
		mcall_free_pma(regs->a0);
		break;
	case SBI_EXT_ANDES_PROBE_PMA:
		*out_value = ((csr_read(CSR_MMSC_CFG) & 0x40000000) != 0);
		break;
	case SBI_EXT_ANDES_DCACHE_WBINVAL_ALL:
		ret = mcall_dcache_wbinval_all();
		break;
	default:
		sbi_printf("Unsupported vendor sbi call : %ld\n", funcid);
		asm volatile("ebreak");
	}
	return ret;
}

/* Platform descriptor. */
const struct sbi_platform_operations platform_ops = {
	.early_init = ae350_early_init,

	.final_init = ae350_final_init,

	.console_init = ae350_console_init,

	.irqchip_init = ae350_irqchip_init,

	.ipi_init     = ae350_ipi_init,

	.timer_init	   = ae350_timer_init,

	.vendor_ext_provider = ae350_vendor_ext_provider
};

struct sbi_platform platform = {
	.opensbi_version = OPENSBI_VERSION,
	.platform_version = SBI_PLATFORM_VERSION(0x0, 0x01),
	.name = "Andes AE350",
	.features = SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count = AE350_HART_COUNT_MAX,
	.hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr = (unsigned long)&platform_ops
};
