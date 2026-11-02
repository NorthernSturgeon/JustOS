/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __FONT_H__
#define __FONT_H__

extern uint8_t* font_symbol_tab[];
extern const size_t font_symbol_tab_size;

extern const uint8_t font_width;
extern const uint8_t font_height;
extern const uint8_t font_full_width;
extern const uint8_t font_full_height;

extern uint8_t *get_symbol_by_id(uint8_t* symbol_tab[], uint8_t sym_id);

#endif