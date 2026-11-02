/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * \file tsv.h
 */
#ifndef __BL_TSV_H__
#define __BL_TSV_H__

extern CHAR8* tsv_init(CHAR8* file, UINTN filesz);
extern CHAR8* tsv_search(CHAR8* base, char* key, UINTN idx);
extern INTN tsv_parseint(CHAR8* base);
extern CHAR16* makechar16(CHAR8* base);

#endif