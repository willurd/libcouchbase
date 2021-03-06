/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include "internal.h"

/**
 * This file contains multiple utility functions that some obscure platforms
 * don't supply
 */


#if !defined(HAVE_HTONLL) && !defined(WORDS_BIGENDIAN)
extern uint64_t libcouchbase_byteswap64(uint64_t val)
{
    size_t ii;
    uint64_t ret = 0;
    for (ii = 0; ii < sizeof(uint64_t); ii++) {
        ret <<= 8;
        ret |= val & 0xff;
        val >>= 8;
    }
    return ret;
}
#endif

