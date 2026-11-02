/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __TLS_H__
#define __TLS_H__

#ifdef __cplusplus
extern "C" {
#endif
/*
__export void* __attribute__((weak)) _DTV_;
extern void* _percpu_start_;

static inline void* tls_ref(size_t off){
	register void* ptr;
	asm (
		"movq _DTV_@GOTPCREL(%%rip), %0 \n\t"
		"movq (%0), %0 \n\t"
		"movq %%gs:(%0), %0 \n\t"
		:"=r"(ptr)::
	);
	return ptr + off - (size_t)&_percpu_start_;
}

#define tls_ptr(var) (&(var))
*/
#define __tls __thread __attribute__((tls_model("local-dynamic")))

#define preempt_disable() OPT_BARRIER()
#define preempt_enable() OPT_BARRIER()

#ifdef __cplusplus
}
#endif

#endif