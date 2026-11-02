/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

.global _inb
.global _outb
.text
    _inb:
        mov %di, %dx
        in %dx, %al
        ret

    _outb:
        mov %di, %dx
        mov %sil, %al
        out %al, %dx
        ret
