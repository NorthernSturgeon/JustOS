/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __MASK_H__
#define __MASK_H__

#define switch_on(val, mask) (val) | (mask)
#define switch_off(val, mask) (val) & ~(mask)

#endif
