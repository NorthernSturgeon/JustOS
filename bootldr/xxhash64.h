/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef XXHASH64_H
#define XXHASH64_H

/**
 * \file xxhash64.h
 */

/**
 Single-function data handler of XXHash64 (see xxhash64.с)
*/
uint64_t xxhash64(const void* data, uint64_t length, uint64_t seed);

#endif