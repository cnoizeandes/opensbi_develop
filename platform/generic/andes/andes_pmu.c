// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023 Andes Technology Corporation
 */
#include <andes/andes45.h>
#include <andes/andes_pmu.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_pmu.h>
#include <sbi/sbi_scratch.h>

static void andes_hw_counter_enable_irq(uint32_t ctr_idx)
{
	unsigned long mip_val;

	if (ctr_idx >= SBI_PMU_HW_CTR_MAX)
		return;

	mip_val = csr_read(CSR_MIP);
	if (!(mip_val & MIP_PMOVI))
		csr_clear(CSR_MCOUNTEROVF, BIT(ctr_idx));

	csr_set(CSR_MCOUNTERINTEN, BIT(ctr_idx));
}

static void andes_hw_counter_disable_irq(uint32_t ctr_idx)
{
	csr_clear(CSR_MCOUNTERINTEN, BIT(ctr_idx));
}

static void andes_hw_counter_filter_mode(unsigned long flags, int ctr_idx)
{
	if (flags & SBI_PMU_CFG_FLAG_SET_UINH)
		csr_set(CSR_MCOUNTERMASK_U, BIT(ctr_idx));
	else
		csr_clear(CSR_MCOUNTERMASK_U, BIT(ctr_idx));

	if (flags & SBI_PMU_CFG_FLAG_SET_SINH)
		csr_set(CSR_MCOUNTERMASK_S, BIT(ctr_idx));
	else
		csr_clear(CSR_MCOUNTERMASK_S, BIT(ctr_idx));
}

static struct sbi_pmu_device andes_pmu = {
	.name = "andes_pmu",
	.hw_counter_enable_irq  = andes_hw_counter_enable_irq,
	.hw_counter_disable_irq = andes_hw_counter_disable_irq,
	/*
	 * We set delegation of supervisor local interrupts via
	 * 18th bit on mslideleg instead of mideleg, so leave
	 * hw_counter_irq_bit() callback unimplemented.
	 */
	.hw_counter_irq_bit     = NULL,
	.hw_counter_filter_mode = andes_hw_counter_filter_mode
};

int andes_pmu_init(void)
{
	if (!has_andes_pmu())
		return SBI_ENOTSUPP;

	/*
	 * It is not reasonable for an Andes CPU to support
	 * both Andes PMU and standard Sscofpmf, as they
	 * serve the same purpose.
	 */
	if (sbi_hart_has_extension(sbi_scratch_thishart_ptr(),
				   SBI_HART_EXT_SSCOFPMF))
		sbi_hart_hang();

	/* Inhibit HPM counter in M-mode */
	csr_write(CSR_MCOUNTERMASK_M, 0xfffffffd);
	/* Delegate S-mode local interrupt to S-mode */
	csr_write(CSR_MSLIDELEG, MIP_PMOVI);

	sbi_pmu_set_device(&andes_pmu);

	return 0;
}
