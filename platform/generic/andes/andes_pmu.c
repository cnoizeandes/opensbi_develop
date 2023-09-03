// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2022 Andes Technology Corporation
 *
 */
#include <andes/andes45.h>
#include <andes/andes_pmu.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_error.h>
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
	if (!flags) {
		csr_write(CSR_MCOUNTERMASK_S, 0);
		csr_write(CSR_MCOUNTERMASK_U, 0);
	}
	if (flags & SBI_PMU_CFG_FLAG_SET_UINH) {
		csr_clear(CSR_MCOUNTERMASK_S, BIT(ctr_idx));
		csr_set(CSR_MCOUNTERMASK_U, BIT(ctr_idx));
	}
	if (flags & SBI_PMU_CFG_FLAG_SET_SINH) {
		csr_set(CSR_MCOUNTERMASK_S, BIT(ctr_idx));
		csr_clear(CSR_MCOUNTERMASK_U, BIT(ctr_idx));
	}
}

static struct sbi_pmu_device andes_pmu = {
	.name = "andes_pmu",
	.hw_counter_enable_irq  = andes_hw_counter_enable_irq,
	.hw_counter_disable_irq = andes_hw_counter_disable_irq,
	/*
	 * In andes_pmu_extensions_init(), we set mslideleg[18] for each
	 * hart instead of mideleg, so leave hw_counter_irq_bit() hook
	 * unimplemented.
	 */
	.hw_counter_irq_bit     = NULL,
	.hw_counter_filter_mode = andes_hw_counter_filter_mode
};

int andes_pmu_extensions_init(const struct fdt_match *match,
			      struct sbi_hart_features *hfeatures)
{
	if (andes_pmu()) {
		/*
		 * It is not rational for a hardware to support
		 * both Andes PMU and standard Sscofpmf, as they
		 * serve the same purpose.
		 */
		if (sbi_hart_has_extension(sbi_scratch_thishart_ptr(),
					   SBI_HART_EXT_SSCOFPMF))
			ebreak();

		/* Machine counter write enable */
		csr_write(CSR_MCOUNTERWEN, 0xfffffffd);
		/* disable HPM counter in M-mode */
		csr_write(CSR_MCOUNTERMASK_M, 0xfffffffd);
		/* delegate S-mode local interrupt to S-mode */
		csr_write(CSR_MSLIDELEG, MIP_PMOVI);

		sbi_pmu_set_device(&andes_pmu);
	}

	return 0;
}
