/* *****************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-02
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * - Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ****************************************************************************/
/*!
 * \file
 * \brief String to integer and integer to string conversion functions.
 */

/* **********************************************************************
 * Purpose: This file contains string to integer and integer to string
 * conversion functions
 * **********************************************************************/

#include <strintmap.hpp>
#include <membuffer.hpp>

int map_str_to_int(const char* name, size_t name_len, str_int_entry* table,
                   int num_entries, int case_sensitive) {
    int top, mid, bot;
    int cmp;
    memptr name_ptr;

    name_ptr.buf = (char*)name;
    name_ptr.length = name_len;

    top = 0;
    bot = num_entries - 1;

    while (top <= bot) {
        mid = (top + bot) / 2;
        if (case_sensitive) {
            /*cmp = strcmp( name, table[mid].name ); */
            cmp = memptr_cmp(&name_ptr, table[mid].name);
        } else {
            /*cmp = strcasecmp( name, table[mid].name ); */
            cmp = memptr_cmp_nocase(&name_ptr, table[mid].name);
        }

        if (cmp > 0) {
            top = mid + 1; /* look below mid */
        } else if (cmp < 0) {
            bot = mid - 1; /* look above mid */
        } else             /* cmp == 0 */
        {
            return mid; /* match; return table index */
        }
    }

    return -1; /* header name not found */
}

int map_int_to_str(int id, str_int_entry* table, int num_entries) {
    int i;

    for (i = 0; i < num_entries; i++) {
        if (table[i].id == id) {
            return i;
        }
    }
    return -1;
}
