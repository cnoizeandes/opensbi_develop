/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Andes Technology Corporation
 *
 * Authors:
 *   Zong Li <zong@andestech.com>
 *   Nylon Chen <nylon7@andestech.com>
 */

#ifndef _AE350_PLATFORM_H_
#define _AE350_PLATFORM_H_

#define AE350_HART_COUNT        4

#define AE350_PLIC_ADDR         0xe4000000
#define AE350_PLIC_NUM_SOURCES      71

#define AE350_PLICSW_ADDR       0xe6400000

#define AE350_PLMT_ADDR         0xe6000000

#define AE350_L2C_ADDR          0xe0500000

#define AE350_UART_ADDR_OFFSET      0x20
#define AE350_UART_ADDR         (0xf0300000 + AE350_UART_ADDR_OFFSET)
#define AE350_UART_FREQUENCY        19660800
#define AE350_UART_BAUDRATE     38400
#define AE350_UART_REG_SHIFT        2
#define AE350_UART_REG_WIDTH        0

/***********************************
 * AndeStar V5 machine mode CSRs
 **********************************/

/* Configuration Registers */
#define CSR_MICM_CFG        0xfc0
#define CSR_MDCM_CFG        0xfc1
#define CSR_MMSC_CFG        0xfc2
#define CSR_MMSC_CFG2       0xfc3
#define CSR_MVEC_CFG        0xfc7

/* Crash Debug CSRs */
#define CSR_MCRASH_STATESAVE    0xfc8
#define CSR_MSTATUS_CRASHSAVE   0xfc9

/* Memory CSRs */
#define CSR_MILMB           0x7c0
#define CSR_MDLMB           0x7c1
#define CSR_MECC_CODE       0x7c2
#define CSR_MNVEC           0x7c3
#define CSR_MCACHE_CTL      0x7ca
#define CSR_MCCTLBEGINADDR  0x7cb
#define CSR_MCCTLCOMMAND    0x7cc
#define CSR_MCCTLDATA       0x7cd
#define CSR_MPPIB           0x7f0
#define CSR_MFIOB           0x7f1

/* Hardware Stack Protection & Recording */
#define CSR_MHSP_CTL        0x7c6
#define CSR_MSP_BOUND       0x7c7
#define CSR_MSP_BASE        0x7c8

/* Trap related CSR */
#define CSR_MXSTATUS        0x7c4
#define CSR_MDCAUSE         0x7c9
#define CSR_MSLIDELEG       0x7d5
#define CSR_MSAVESTATUS     0x7d6
#define CSR_MSAVEEPC1       0x7d7
#define CSR_MSAVECAUSE1     0x7d8
#define CSR_MSAVEEPC2       0x7d9
#define CSR_MSAVECAUSE2     0x7da
#define CSR_MSAVEDCAUSE1    0x7db
#define CSR_MSAVEDCAUSE2    0x7dc

/* Control CSRs */
#define CSR_MPFT_CTL        0x7c5
#define CSR_MMISC_CTL       0x7d0
#define CSR_MCLK_CTL        0x7df

/* Counter related CSRs */
#define CSR_MCOUNTERWEN        0x7ce
#define CSR_MCOUNTERINTEN      0x7cf
#define CSR_MCOUNTERMASK_M     0x7d1
#define CSR_MCOUNTERMASK_S     0x7d2
#define CSR_MCOUNTERMASK_U     0x7d3
#define CSR_MCOUNTEROVF        0x7d4

/***********************************
 * AndeStar V5 supervisor mode CSRs
 **********************************/

/* Supervisor Trap Related */
#define CSR_SLIE            0x9c4
#define CSR_SLIP            0x9c5
#define CSR_SDCAUSE         0x9c9

/* Supervisor Counter Related */
#define CSR_SCOUNTERINTEN   0x9cf
#define CSR_SCOUNTERMASK_M  0x9d1
#define CSR_SCOUNTERMASK_S  0x9d2
#define CSR_SCOUNTERMASK_U  0x9d3
#define CSR_SCOUNTEROVF     0x9d4
#define CSR_SCOUNTINHIBIT   0x9e0
#define CSR_SHPMEVENT3      0x9e3
#define CSR_SHPMEVENT4      0x9e4
#define CSR_SHPMEVENT5      0x9e5
#define CSR_SHPMEVENT6      0x9e6

/* Supervisor Control */
#define CSR_SCCTLDATA       0x9cd
#define CSR_SMISC_CTL       0x9d0

/***********************************
 * AndeStar V5 user mode CSRs
 **********************************/
#define CSR_UITB            0x800
#define CSR_UCODE           0x801
#define CSR_UDCAUSE         0x809
#define CSR_UCCTLBEGINADDR  0x80b
#define CSR_UCCTLCOMMAND    0x80c
#define CSR_WFE             0x810
#define CSR_SLEEPVALUE      0x811
#define CSR_TXEVT           0x812

/* #define ANDES_SBI_PLATFORM_DEFAULT_FEATURES \
 *		(SBI_PLATFORM_HAS_TIMER_VALUE |\
 *		SBI_PLATFORM_HAS_HART_HOTPLUG |\
 *		SBI_PLATFORM_HAS_MFAULTS_DELEGATION |\
 *		SBI_PLATFORM_HAS_HART_SECONDARY_BOOT)
 */

#ifndef __ASSEMBLY__
enum sbi_ext_andes_fid {
	SBI_EXT_ANDES_GET_MCACHE_CTL_STATUS = 0,
	SBI_EXT_ANDES_GET_MMISC_CTL_STATUS,
	SBI_EXT_ANDES_SET_MCACHE_CTL,
	SBI_EXT_ANDES_SET_MMISC_CTL,
	SBI_EXT_ANDES_ICACHE_OP,
	SBI_EXT_ANDES_DCACHE_OP,
	SBI_EXT_ANDES_L1CACHE_I_PREFETCH,
	SBI_EXT_ANDES_L1CACHE_D_PREFETCH,
	SBI_EXT_ANDES_NON_BLOCKING_LOAD_STORE,
	SBI_EXT_ANDES_WRITE_AROUND,
	SBI_EXT_ANDES_TRIGGER,
	SBI_EXT_ANDES_SET_PFM,
	SBI_EXT_ANDES_READ_POWERBRAKE,
	SBI_EXT_ANDES_WRITE_POWERBRAKE,
	SBI_EXT_ANDES_SUSPEND_PREPARE,
	SBI_EXT_ANDES_SUSPEND_MEM,
	SBI_EXT_ANDES_SET_SUSPEND_MODE,
	SBI_EXT_ANDES_ENTER_SUSPEND_MODE,
	SBI_EXT_ANDES_RESTART,
	SBI_EXT_ANDES_RESET_VEC,
	SBI_EXT_ANDES_SET_PMA,
	SBI_EXT_ANDES_FREE_PMA,
	SBI_EXT_ANDES_PROBE_PMA,
	SBI_EXT_ANDES_DCACHE_WBINVAL_ALL,
};
#endif

/* MISC */
#define IRQ_PFM         18
#define MIP_MOVFIP      (1 << IRQ_PFM)
#define MIP_SOVFIP      (1 << IRQ_PFM)

/* nds v5 mmisc_ctl register*/
#define V5_MMISC_CTL_VEC_PLIC_OFFSET            1
#define V5_MMISC_CTL_RVCOMPM_OFFSET             2
#define V5_MMISC_CTL_BRPE_OFFSET                3
#define V5_MMISC_CTL_MSA_OR_UNA_OFFSET          6
#define V5_MMISC_CTL_NON_BLOCKING_OFFSET        8

#define V5_MMISC_CTL_VEC_PLIC_EN        (1UL << V5_MMISC_CTL_VEC_PLIC_OFFSET)
#define V5_MMISC_CTL_RVCOMPM_EN         (1UL << V5_MMISC_CTL_RVCOMPM_OFFSET)
#define V5_MMISC_CTL_BRPE_EN            (1UL << V5_MMISC_CTL_BRPE_OFFSET)
#define V5_MMISC_CTL_MSA_OR_UNA_EN      (1UL << V5_MMISC_CTL_MSA_OR_UNA_OFFSET)
#define V5_MMISC_CTL_NON_BLOCKING_EN    (1UL << V5_MMISC_CTL_NON_BLOCKING_OFFSET)

#define V5_MMISC_CTL_MASK  (V5_MMISC_CTL_VEC_PLIC_EN | V5_MMISC_CTL_RVCOMPM_EN \
	| V5_MMISC_CTL_BRPE_EN | V5_MMISC_CTL_MSA_OR_UNA_EN | V5_MMISC_CTL_NON_BLOCKING_EN)

/* nds mcache_ctl register*/
#define V5_MCACHE_CTL_IC_EN_OFFSET      0
#define V5_MCACHE_CTL_DC_EN_OFFSET      1
#define V5_MCACHE_CTL_IC_ECCEN_OFFSET   2
#define V5_MCACHE_CTL_DC_ECCEN_OFFSET   4
#define V5_MCACHE_CTL_IC_RWECC_OFFSET   6
#define V5_MCACHE_CTL_DC_RWECC_OFFSET   7
#define V5_MCACHE_CTL_CCTL_SUEN_OFFSET  8
#define V5_MCACHE_CTL_L1I_PREFETCH_OFFSET       9
#define V5_MCACHE_CTL_L1D_PREFETCH_OFFSET       10
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_1       13
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_2       14
#define V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1      15
#define V5_MCACHE_CTL_L2C_WAROUND_OFFSET_2      16

#define V5_MCACHE_CTL_DC_COHEN_OFFSET     19
#define V5_MCACHE_CTL_DC_COHSTA_OFFSET    20

/*nds cctl command*/
#define V5_UCCTL_L1D_WBINVAL_ALL 6
#define V5_UCCTL_L1D_WB_ALL 7
#define V5_UCCTL_L1D_INVAL_ALL 23

#define V5_MCACHE_CTL_IC_EN     (1UL << V5_MCACHE_CTL_IC_EN_OFFSET)
#define V5_MCACHE_CTL_DC_EN     (1UL << V5_MCACHE_CTL_DC_EN_OFFSET)
#define V5_MCACHE_CTL_IC_RWECC  (1UL << V5_MCACHE_CTL_IC_RWECC_OFFSET)
#define V5_MCACHE_CTL_DC_RWECC  (1UL << V5_MCACHE_CTL_DC_RWECC_OFFSET)
#define V5_MCACHE_CTL_CCTL_SUEN (1UL << V5_MCACHE_CTL_CCTL_SUEN_OFFSET)
#define V5_MCACHE_CTL_L1I_PREFETCH_EN   (1UL << V5_MCACHE_CTL_L1I_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_L1D_PREFETCH_EN   (1UL << V5_MCACHE_CTL_L1D_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_DC_WAROUND_1_EN   (1UL << V5_MCACHE_CTL_DC_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_DC_WAROUND_2_EN   (1UL << V5_MCACHE_CTL_DC_WAROUND_OFFSET_2)
#define V5_MCACHE_CTL_L2C_WAROUND_1_EN   (1UL << V5_MCACHE_CTL_L2C_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_L2C_WAROUND_2_EN   (1UL << V5_MCACHE_CTL_L2C_WAROUND_OFFSET_2)
#define V5_MCACHE_CTL_DC_COHEN_EN       (1UL << V5_MCACHE_CTL_DC_COHEN_OFFSET)
#define V5_MCACHE_CTL_DC_COHSTA_EN      (1UL << V5_MCACHE_CTL_DC_COHSTA_OFFSET)

#define V5_MCACHE_CTL_MASK (V5_MCACHE_CTL_IC_EN | V5_MCACHE_CTL_DC_EN \
	| V5_MCACHE_CTL_IC_RWECC | V5_MCACHE_CTL_DC_RWECC \
	| V5_MCACHE_CTL_CCTL_SUEN | V5_MCACHE_CTL_L1I_PREFETCH_EN \
	| V5_MCACHE_CTL_L1D_PREFETCH_EN | V5_MCACHE_CTL_DC_WAROUND_1_EN \
	| V5_MCACHE_CTL_DC_WAROUND_2_EN)

#define V5_L2C_CTL_OFFSET           0x8
#define V5_L2C_CTL_ENABLE_OFFSET    0
#define V5_L2C_CTL_IPFDPT_OFFSET    3
#define V5_L2C_CTL_DPFDPT_OFFSET    5
#define V5_L2C_CTL_TRAMOCTL_OFFSET  8
#define V5_L2C_CTL_TRAMICTL_OFFSET  10
#define V5_L2C_CTL_DRAMOCTL_OFFSET  11
#define V5_L2C_CTL_DRAMICTL_OFFSET  13

#define V5_L2C_CTL_ENABLE_MASK      (1UL << V5_L2C_CTL_ENABLE_OFFSET)
#define V5_L2C_CTL_IPFDPT_MASK      (3UL << V5_L2C_CTL_IPFDPT_OFFSET)
#define V5_L2C_CTL_DPFDPT_MASK      (3UL << V5_L2C_CTL_DPFDPT_OFFSET)
#define V5_L2C_CTL_TRAMOCTL_MASK    (3UL << V5_L2C_CTL_TRAMOCTL_OFFSET)
#define V5_L2C_CTL_TRAMICTL_MASK    (1UL << V5_L2C_CTL_TRAMICTL_OFFSET)
#define V5_L2C_CTL_DRAMOCTL_MASK    (3UL << V5_L2C_CTL_DRAMOCTL_OFFSET)
#define V5_L2C_CTL_DRAMICTL_MASK    (1UL << V5_L2C_CTL_DRAMICTL_OFFSET)

#ifndef __ASSEMBLY__
extern int ae350_suspend_mode[];
int ae350_enter_suspend_mode(int suspend_mode, bool main_core,
				unsigned int wake_mask, int num_cpus);
static inline __attribute__((always_inline)) bool is_andestar45_series(void)
{
	uintptr_t marchid = csr_read(CSR_MARCHID);
	return ((marchid & 0xF0) >> 4 == 4 &&
			(marchid & 0xF) == 5) ? true : false;
}
#endif /* __ASSEMBLY__ */

#endif /* _AE350_PLATFORM_H_ */
