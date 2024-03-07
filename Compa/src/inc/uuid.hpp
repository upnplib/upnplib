#ifndef COMPA_UUID_HPP
#define COMPA_UUID_HPP
/*
 * Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
 * Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
 * Digital Equipment Corporation, Maynard, Mass.
 * Copyright (c) 1998 Microsoft.
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-08
 *
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty: permission to use, copy,
 * modify, and distribute this file for any purpose is hereby
 * granted without fee, provided that the above copyright notices and
 * this notice appears in all source code copies, and that none of
 * the names of Open Software Foundation, Inc., Hewlett-Packard
 * Company, or Digital Equipment Corporation be used in advertising
 * or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Neither Open Software
 * Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital Equipment
 * Corporation makes any representations about the suitability of
 * this software for any purpose.
 */
/*!
 * \file
 * \ingroup uuid
 * \brief Manage UUIDs
 */

#include <sysdep.hpp>

/*! \brief uuid UPNP */
struct uuid_upnp {
    /// @{
    /// \brief Member variable
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t clock_seq_hi_and_reserved;
    uint8_t clock_seq_low;
    uint8_t node[6];
    /// @}
};

/*!
 * \brief Generate a UUID.
 *
 * \returns Always **1**.
 */
int uuid_create(   //
    uuid_upnp* uid ///< [out] Pointer to a place for the created uuid.
);

/*!
 * \brief Unpack a UUID.
 */
void upnp_uuid_unpack( //
    uuid_upnp* u,      ///< [in] Packed UUID.
    char* out          ///< [out] Will be xxxx-xx-xx-xx-xxxxxx format.
);

/*!
 * \brief Create a UUID using a "name" from a "name space".
 */
void uuid_create_from_name(
    /*! [out] Resulting UUID. */
    uuid_upnp* uid,
    /*! [in] UUID to serve as context, so identical names from different name
     * spaces generate different UUIDs. */
    uuid_upnp nsid,
    /*! [in] The name from which to generate a UUID. */
    void* name,
    /*! [in] The length of the name. */
    int namelen);

/*!
 * \brief Compare two UUID's "lexically".
 *
 * \returns
 *  -      **-1**: u1 is lexically before u2
 *  - &nbsp;**0**: u1 is equal to u2
 *  - &nbsp;**1**: u1 is lexically after u2
 *
 * \note Lexical ordering is not temporal ordering!
 */
int uuid_compare(  //
    uuid_upnp* u1, ///< [in]
    uuid_upnp* u2  ///< [in]
);

#endif /* COMPA_UUID_HPP */
