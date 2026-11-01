/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __FONT_H__
#define __FONT_H__

typedef unsigned char uint8_t;

extern void* font_symbol_table[];
extern size_t font_symbol_table_size;

extern uint16_t font_width;
extern uint16_t font_height;
extern uint16_t font_full_width;
extern uint16_t font_full_height;

extern uint8_t* get_symbol_by_id(uint8_t sym_id);

#endif