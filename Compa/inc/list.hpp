#ifndef COMPA_LIST_HPP
#define COMPA_LIST_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2011-2012 France Telecom All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-02-13
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
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
 ******************************************************************************/
/*!
 * \file
 * \brief Trivial list management interface, patterned on std::list.
 *
 * It aims more at being familiar than at being minimal. The implementation does
 * not perform any allocation or deallocation.
 */

#include <upnplib/visibility.hpp>

/*! \brief List anchor structure.
 *
 * This should be the *first* entry in list member objects, except if you want
 * to do member offset arithmetic instead of simple casts (look up
 * "containerof"). The list code itself does not care. */
struct UpnpListHead {
    UpnpListHead* next; ///< Points to next entry in the list
    UpnpListHead* prev; ///< Points to previous entry in the list
};

/// \brief List iterator. Not strictly necessary, but clarifies the interface.
typedef UpnpListHead* UpnpListIter;

/*! \brief Initialize empty list */
UPNPLIB_API void UpnpListInit(UpnpListHead* list);

/*! \brief Return iterator pointing to the first list element, or
 *  UpnpListEnd(list) if the list is empty */
UPNPLIB_API UpnpListIter UpnpListBegin(UpnpListHead* list);

/*! \brief Return end of list sentinel iterator (not an element) */
UPNPLIB_API UpnpListIter UpnpListEnd(UpnpListHead* list);

/*! \brief Return iterator pointing to element after pos, or end() */
UPNPLIB_API UpnpListIter UpnpListNext(UpnpListHead* list, UpnpListIter pos);

/*! \brief Insert element before pos, returns iterator pointing to inserted
 * element. */
UPNPLIB_API UpnpListIter UpnpListInsert(UpnpListHead* list, UpnpListIter pos,
                                        UpnpListHead* elt);

/*! \brief Erase element at pos, return next one, or end()*/
UPNPLIB_API UpnpListIter UpnpListErase(UpnpListHead* list, UpnpListIter pos);

#endif // COMPA_LIST_HPP
