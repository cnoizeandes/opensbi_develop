// SPDX-License-Identifier: BSD-2-Clause

#ifndef _RISCV_ANDES_PMU_H
#define _RISCV_ANDES_PMU_H

#include <sbi/sbi_hart.h>
#include <sbi_utils/fdt/fdt_helper.h>

int andes_pmu_extensions_init(const struct fdt_match *match,
			      struct sbi_hart_features *hfeatures);

#endif /* _RISCV_ANDES_PMU_H */
