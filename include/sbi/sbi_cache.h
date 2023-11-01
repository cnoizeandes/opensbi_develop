/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */

#ifndef __SBI_CACHE_H__
#define __SBI_CACHE_H__

#include <sbi/sbi_types.h>
#include <sbi/sbi_hartmask.h>
#include <../platform/andes/ae350/cache.h>

#define SBI_CMO 0x53434D4F	// ascii: SCMO

/* clang-format on */

struct sbi_scratch;

/*
 * struct sbi_cache_info has to have identical member number as struct sbi_tlb_info
 * and each member should have the exact same size as struct sbi_tlb_info
 */
struct sbi_cache_info {
	unsigned long start;
	unsigned long size;
	unsigned long extension;
	unsigned long last_hartid;
	void (*local_fn)(struct sbi_cache_info *cinfo);
	struct sbi_hartmask smask;
};

#define SBI_CACHE_INFO_INIT(__p, __start, __size, __dummy, __last_hartid, __lfn, __src) \
do { \
	(__p)->start = (__start); \
	(__p)->size = (__size); \
	(__p)->extension = SBI_CMO; \
	(__p)->last_hartid = __last_hartid; \
	(__p)->local_fn = (__lfn); \
	SBI_HARTMASK_INIT_EXCEPT(&(__p)->smask, (__src)); \
} while (0)

void sbi_cache_invalidate_line(struct sbi_cache_info *cinfo);
void sbi_cache_invalidate_range(struct sbi_cache_info *cinfo);
void sbi_cache_wb_line(struct sbi_cache_info *cinfo);
void sbi_cache_wb_range(struct sbi_cache_info *cinfo);
void sbi_cache_wbinval_all(struct sbi_cache_info *cinfo);

#endif