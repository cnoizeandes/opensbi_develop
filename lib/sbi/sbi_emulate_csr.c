/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_emulate_csr.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_timer.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_hart.h>

int sbi_emulate_csr_read(int csr_num, struct sbi_trap_regs *regs,
			 ulong *csr_val)
{
	int ret = 0;
	ulong cen = -1UL;
	ulong prev_mode = (regs->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SHIFT;
#if __riscv_xlen == 32
	bool virt = (regs->mstatusH & MSTATUSH_MPV) ? TRUE : FALSE;
#else
	bool virt = (regs->mstatus & MSTATUS_MPV) ? TRUE : FALSE;
#endif

	if (prev_mode == PRV_U){
		cen = csr_read(CSR_SCOUNTEREN);
		if (csr_num == CSR_TIME &&
		    !sbi_hart_has_feature(sbi_scratch_thishart_ptr(),
					  SBI_HART_HAS_TIME))
			cen |= (1UL << 1);
	}

	switch (csr_num) {
	case CSR_HTIMEDELTA:
		if (prev_mode == PRV_S && !virt)
			*csr_val = sbi_timer_get_delta();
		else
			ret = SBI_ENOTSUPP;
		break;
	case CSR_CYCLE:
		if (!((cen >> (CSR_CYCLE - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MCYCLE);
		break;
	case CSR_TIME:
		if (!((cen >> (CSR_TIME - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = (virt) ? sbi_timer_virt_value():
				    sbi_timer_value();
		break;
	case CSR_INSTRET:
		if (!((cen >> (CSR_INSTRET - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MINSTRET);
		break;
	case CSR_MHPMCOUNTER3:
		if (!((cen >> (3 + CSR_MHPMCOUNTER3 - CSR_MHPMCOUNTER3)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MHPMCOUNTER3);
		break;
	case CSR_MHPMCOUNTER4:
		if (!((cen >> (3 + CSR_MHPMCOUNTER4 - CSR_MHPMCOUNTER3)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MHPMCOUNTER4);
		break;
#if __riscv_xlen == 32
	case CSR_HTIMEDELTAH:
		if (prev_mode == PRV_S && !virt)
			*csr_val = sbi_timer_get_delta() >> 32;
		else
			ret = SBI_ENOTSUPP;
		break;
	case CSR_CYCLEH:
		if (!((cen >> (CSR_CYCLE - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MCYCLEH);
		break;
	case CSR_TIMEH:
		if (!((cen >> (CSR_TIME - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = (virt) ? sbi_timer_virt_value() >> 32:
				    sbi_timer_value() >> 32;
		break;
	case CSR_INSTRETH:
		if (!((cen >> (CSR_INSTRET - CSR_CYCLE)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MINSTRETH);
		break;
	case CSR_MHPMCOUNTER3H:
		if (!((cen >> (3 + CSR_MHPMCOUNTER3 - CSR_MHPMCOUNTER3)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MHPMCOUNTER3H);
		break;
	case CSR_MHPMCOUNTER4H:
		if (!((cen >> (3 + CSR_MHPMCOUNTER4 - CSR_MHPMCOUNTER3)) & 1))
			return -1;
		*csr_val = csr_read(CSR_MHPMCOUNTER4H);
		break;
#endif
	case CSR_MHPMEVENT3:
		*csr_val = csr_read(CSR_MHPMEVENT3);
		break;
	case CSR_MHPMEVENT4:
		*csr_val = csr_read(CSR_MHPMEVENT4);
		break;
	default:
		ret = SBI_ENOTSUPP;
		break;
	};

	if (ret)
		sbi_dprintf("%s: hartid%d: invalid csr_num=0x%x\n",
			    __func__, current_hartid(), csr_num);

	return ret;
}

int sbi_emulate_csr_write(int csr_num, struct sbi_trap_regs *regs,
			  ulong csr_val)
{
	int ret = 0;
	ulong prev_mode = (regs->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SHIFT;
#if __riscv_xlen == 32
	bool virt = (regs->mstatusH & MSTATUSH_MPV) ? TRUE : FALSE;
#else
	bool virt = (regs->mstatus & MSTATUS_MPV) ? TRUE : FALSE;
#endif

	switch (csr_num) {
	case CSR_HTIMEDELTA:
		if (prev_mode == PRV_S && !virt)
			sbi_timer_set_delta(csr_val);
		else
			ret = SBI_ENOTSUPP;
		break;
	case CSR_CYCLE:
		csr_write(CSR_MCYCLE, csr_val);
		break;
	case CSR_INSTRET:
		csr_write(CSR_MINSTRET, csr_val);
		break;
	case CSR_MHPMCOUNTER3:
		csr_write(CSR_MHPMCOUNTER3, csr_val);
		break;
	case CSR_MHPMCOUNTER4:
		csr_write(CSR_MHPMCOUNTER4, csr_val);
		break;
#if __riscv_xlen == 32
	case CSR_HTIMEDELTAH:
		if (prev_mode == PRV_S && !virt)
			sbi_timer_set_delta_upper(csr_val);
		else
			ret = SBI_ENOTSUPP;
		break;
	case CSR_CYCLEH:
		csr_write(CSR_MCYCLEH, csr_val);
		break;
	case CSR_INSTRETH:
		csr_write(CSR_MINSTRETH, csr_val);
		break;
	case CSR_MHPMCOUNTER3H:
		csr_write(CSR_MHPMCOUNTER3H, csr_val);
		break;
	case CSR_MHPMCOUNTER4H:
		csr_write(CSR_MHPMCOUNTER4H, csr_val);
		break;
#endif
	case CSR_MHPMEVENT3:
		csr_write(CSR_MHPMEVENT3, csr_val);
		break;
	case CSR_MHPMEVENT4:
		csr_write(CSR_MHPMEVENT4, csr_val);
		break;
	default:
		ret = SBI_ENOTSUPP;
		break;
	};

	if (ret)
		sbi_dprintf("%s: hartid%d: invalid csr_num=0x%x\n",
			    __func__, current_hartid(), csr_num);

	return ret;
}
