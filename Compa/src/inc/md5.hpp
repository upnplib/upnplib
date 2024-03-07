/*  $OpenBSD: md5.h,v 1.3 2014/11/16 17:39:09 tedu Exp $    */

/*
 * Last modified: 2024-02-01 --Ingo
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

/*!
 * \file
 * \ingroup uuid
 * \brief This code implements the MD5 message-digest algorithm.
 *
 * The algorithm is due to Ron Rivest. This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 */

#ifndef UPNPLIB_MD5_HPP
#define UPNPLIB_MD5_HPP

#include <stddef.h>
#include <stdint.h>

/// \cond
#define MD5_BLOCK_LENGTH 64
#define MD5_DIGEST_LENGTH 16
/// \endcond

/// \brief MD5 Context
struct MD5_CTX {
    /// \cond
    uint32_t state[4];                /* state */
    uint64_t count;                   /* number of bits, mod 2^64 */
    uint8_t buffer[MD5_BLOCK_LENGTH]; /* input buffer */
    /// \endcond
};

/// \brief MD5 Initialisation
void MD5Init(MD5_CTX*);
/// \brief MD5 Update
void MD5Update(MD5_CTX*, const void*, size_t);
/// MD5 Final
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5_CTX* ctx);
/// MD5 Transform
void MD5Transform(uint32_t state[4], const uint8_t block[MD5_BLOCK_LENGTH]);

#endif /* UPNPLIB_MD5_HPP */
