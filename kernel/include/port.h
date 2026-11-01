/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __PORT_H__
#define __PORT_H__

static inline uint8_t port_in8(uint16_t port){
	uint8_t ret;
	asm ("insb \n\t"::"+D"(&ret),"d"(port));
	return ret;
}

static inline uint16_t port_in16(uint16_t port){
	uint16_t ret;
	asm ("insw \n\t"::"+D"(&ret),"d"(port));
	return ret;
}

static inline uint32_t port_in32(uint16_t port){
	uint32_t ret;
	asm ("insl \n\t"::"+D"(&ret),"d"(port));
	return ret;
}

static inline void port_out8(uint16_t port, uint8_t val){
	asm ("outsb \n\t"::"S"(&val),"d"(port));
}

static inline void port_out16(uint16_t port, uint16_t val){
	asm ("outsw \n\t"::"S"(&val),"d"(port));
}

static inline void port_out32(uint16_t port, uint32_t val){
	asm ("outsl \n\t"::"S"(&val),"d"(port));
}

#endif
