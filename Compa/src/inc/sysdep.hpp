#ifndef COMPA_SYSDEP_HPP
#define COMPA_SYSDEP_HPP
/*
 * Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-08
 * Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
 * Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
 * Digital Equipment Corporation, Maynard, Mass.
 * Copyright (c) 1998 Microsoft.
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
// Last compare with pupnp original source file on 2023-07-08, ver 1.14.17

/*!
 * \file
 * \ingroup uuid
 * \brief Get some system dependent information.
 */

#include <ithread.hpp>

/* change to point to where MD5.hpp's live */
/* get MD5 sample implementation from RFC 1321 */
#include <md5.hpp>

#ifdef _WIN32
/* Do not #include <sys/time.h> */
#else
#include <sys/time.h>
#endif

/*! \brief set the following to the number of 100ns ticks of the actual
 * resolution of your system's clock */
inline constexpr int UUIDS_PER_TICK{1024};

/*! \brief Set the following to a call to acquire a system wide global lock. */
extern ithread_mutex_t gUUIDMutex;

/// \brief UUID lock
#define UUIDLock() ithread_mutex_lock(&gUUIDMutex)
/// \brief UUID unlock
#define UUIDUnlock() ithread_mutex_unlock(&gUUIDMutex)

/// \brief UUID time
typedef uint64_t uuid_time_t;

/// \brief node ID
struct uuid_node_t {
    char nodeID[6]; ///< node ID
};

/*!
 * \brief System dependent call to get IEEE node identifier.
 *
 * This sample implementation generates a random node ID.
 */
void get_ieee_node_identifier(uuid_node_t* node);

/*!
 * \brief System dependent call to get the current system time.
 *
 * Returned as 100ns ticks since Oct 15, 1582, but resolution may be
 * less than 100ns.
 */
void get_system_time(uuid_time_t* uuid_time);

/// \brief Get random information.
void get_random_info(unsigned char seed[16]);

#endif /* COMPA_SYSDEP_HPP */
