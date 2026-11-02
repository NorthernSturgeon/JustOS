/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kentry)

SECTIONS{
	. = 0x1000;
	.text ALIGN(4096):{
		*(.text)
	}
	.data ALIGN(4096):{ *(.data) *(.bss)}

	/DISCARD/ :{
		*(.comment)
		*(.note*)
		*(.eh_frame)
	}
}
