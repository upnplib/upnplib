#ifndef COMPA_GENLIB_UTIL_STRINTMAP_HPP
#define COMPA_GENLIB_UTIL_STRINTMAP_HPP
/* *****************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-27
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
 *
 * There are some constant mappings of a number to its string name for human
 * readability or for UPnP messages, e.g. error number, HTTP method etc. Because
 * these functions use very effective binary search, the map tables to seach
 * must be sorted by the key.
 */

#include <upnplib/visibility.hpp>
/// \cond
#include <cstddef> // for size_t
/// \endcond

/// String to integer map entry.
struct str_int_entry {
    const char* name; ///< A value in string form.
    const int id;     ///< Same value in integer form.
};

/*!
 * \brief Match the given name with names from the entries in the table.
 *
 * \returns
 * On success: Zero based index (position) on the table of entries.\n
 * On failure: -1
 */
// Don't export function symbol; only used library intern.
int map_str_to_int(
    const char* name, ///< [in] String containing the name to be matched.
    size_t name_len,  ///< [in] Size of the string to be matched.
    const str_int_entry*
        table,          ///< [in] Table of entries that need to be matched.
    size_t num_entries, /*!< [in] Number of entries in the table that need to be
                                  searched. */
    int case_sensitive ///< [in] Whether search should be case sensitive or not.
);

/*!
 * \brief Returns the index from the table where the id matches the entry from
 * the table.
 *
 * \returns
 * On success: Zero based index (position) on the table of entries.\n
 * On error: -1
 */
// Don't export function symbol; only used library intern.
int map_int_to_str( //
    int id,         ///< [in] ID to be matched.
    const str_int_entry*
        table,      ///< [in] Table of entries that need to be matched.
    int num_entries ///< [in] Number of entries in the table.
);

#endif /* COMPA_GENLIB_UTIL_STRINTMAP_HPP */
