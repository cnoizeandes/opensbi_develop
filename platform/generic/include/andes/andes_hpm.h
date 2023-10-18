/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Andes Technology Corporation
 */

#ifndef _ANDES_HPM_H_
#define _ANDES_HPM_H_

#include <sbi_utils/fdt/fdt_helper.h>

#define ANDES_MHPM_MAP		0x78
#define ANDES_RAW_EVENT_MASK	~0ULL

/* Event code for instruction commit events */
#define ANDES_CYCLES				0x10
#define ANDES_INSTRET				0x20
#define ANDES_INT_LOAD_INST			0x30
#define ANDES_INT_STORE_INST			0x40
#define ANDES_ATOMIC_INST			0x50
#define ANDES_SYS_INST				0x60
#define ANDES_INT_COMPUTE_INST			0x70
#define ANDES_CONDITION_BR			0x80
#define ANDES_TAKEN_CONDITION_BR		0x90
#define ANDES_JAL_INST				0xA0
#define ANDES_JALR_INST				0xB0
#define ANDES_RET_INST				0xC0
#define ANDES_CONTROL_TRANS_INST		0xD0
#define ANDES_EX9_INST				0xE0
#define ANDES_INT_MUL_INST			0xF0
#define ANDES_INT_DIV_REMAINDER_INST		0x100
#define ANDES_FLOAT_LOAD_INST			0x110
#define ANDES_FLOAT_STORE_INST			0x120
#define ANDES_FLOAT_ADD_SUB_INST		0x130
#define ANDES_FLOAT_MUL_INST			0x140
#define ANDES_FLOAT_FUSED_MULADD_INST		0x150
#define ANDES_FLOAT_DIV_SQUARE_ROOT_INST	0x160
#define ANDES_OTHER_FLOAT_INST			0x170
#define ANDES_INT_MUL_AND_SUB_INST		0x180
#define ANDES_RETIRED_OP			0x190

/* Event code for memory system events */
#define ANDES_ILM_ACCESS			0x01
#define ANDES_DLM_ACCESS			0x11
#define ANDES_ICACHE_ACCESS			0x21
#define ANDES_ICACHE_MISS			0x31
#define ANDES_DCACHE_ACCESS			0x41
#define ANDES_DCACHE_MISS			0x51
#define ANDES_DCACHE_LOAD_ACCESS		0x61
#define ANDES_DCACHE_LOAD_MISS			0x71
#define ANDES_DCACHE_STORE_ACCESS		0x81
#define ANDES_DCACHE_STORE_MISS			0x91
#define ANDES_DCACHE_WB				0xA1
#define ANDES_CYCLE_WAIT_ICACHE_FILL		0xB1
#define ANDES_CYCLE_WAIT_DCACHE_FILL		0xC1
#define ANDES_UNCACHED_IFETCH_FROM_BUS		0xD1
#define ANDES_UNCACHED_LOAD_FROM_BUS		0xE1
#define ANDES_CYCLE_WAIT_UNCACHED_IFETCH	0xF1
#define ANDES_CYCLE_WAIT_UNCACHED_LOAD		0x101
#define ANDES_MAIN_ITLB_ACCESS			0x111
#define ANDES_MAIN_ITLB_MISS			0x121
#define ANDES_MAIN_DTLB_ACCESS			0x131
#define ANDES_MAIN_DTLB_MISS			0x141
#define ANDES_CYCLE_WAIT_ITLB_FILL		0x151
#define ANDES_PIPE_STALL_CYCLE_DTLB_MISS	0x161
#define ANDES_HW_PREFETCH_BUS_ACCESS		0x171

/* Event code for microarchitecture events */
#define ANDES_MISPREDICT_CONDITION_BR		0x02
#define ANDES_MISPREDICT_TAKE_CONDITION_BR	0x12
#define ANDES_MISPREDICT_TARGET_RET_INST	0x22

#ifdef CONFIG_ANDES_HPM

int andes_fdt_add_pmu_mappings(void *fdt, const struct fdt_match *match);

#else

int andes_fdt_add_pmu_mappings(void *fdt, const struct fdt_match *match) { return 0; }

#endif /* CONFIG_ANDES_HPM */

#endif /* _ANDES_HPM_H_ */
