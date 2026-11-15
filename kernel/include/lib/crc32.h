/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __CRC32_H__
#define __CRC32_H__

extern __attribute__((weak)) void init_crc32();
extern uint32_t crc32(const uint8_t *buf, size_t len);

#endif