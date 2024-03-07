#ifndef COMPA_UPNPSTRING_HPP
#define COMPA_UPNPSTRING_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-07
// Also Copyright by other contributor as noted below.
// Last compare with pupnp original source file on 2023-04-26, ver 1.14.15
/*!
 * \file
 * \brief UpnpString object declaration.
 */

/*!
 * \defgroup UpnpString The UpnpString Class
 * \brief Implements string operations in the UPnP library.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 * \version 1.0
 * @{
 */

#include <upnplib/visibility.hpp>
/// \cond
#include <cstddef> // For size_t
/// \endcond

/*!
 * \brief Type of the string objects inside libupnp.
 */
// The typedef must be the same as in pupnp otherwise we cannot switch between
// pupnp gtest and compa gtest. Using the typedef in the header file but the
// definiton of the structure in the source file make the mmembers of the
// structure publicy invisible. That is intended but we will change it with
// using C++ private. --Ingo
typedef struct s_UpnpString UpnpString;

/*!
 * \brief Constructor.
 *
 * \return A pointer to a new allocated object.
 */
UPNPLIB_API UpnpString* UpnpString_new();

/*!
 * \brief Destructor.
 */
UPNPLIB_API void UpnpString_delete(
    /*! [in] The \em \b this pointer. */
    UpnpString* p);

/*!
 * \brief Copy Constructor.
 *
 * \return A pointer to a new allocated copy of the original object.
 */
UPNPLIB_API UpnpString* UpnpString_dup(
    /*! [in] The \em \b this pointer. */
    const UpnpString* p);

/*!
 * \brief Assignment operator.
 */
UPNPLIB_API void UpnpString_assign(
    /*! [in] The \em \b this pointer. */
    UpnpString* p,
    /*! [in] The \em \b that pointer. */
    const UpnpString* q);

/*!
 * \brief Returns the length of the string.
 *
 * \return The length of the string.
 * */
UPNPLIB_API size_t UpnpString_get_Length(
    /*! [in] The \em \b this pointer. */
    const UpnpString* p);

/*!
 * \brief Truncates the string to the specified lenght, or does nothing
 * if the current lenght is less than or equal to the requested length.
 * */
UPNPLIB_API void UpnpString_set_Length(
    /*! [in] The \em \b this pointer. */
    UpnpString* p,
    /*! [in] The requested length. */
    size_t n);

/*!
 * \brief Returns the pointer to char.
 *
 * \return The pointer to char.
 * \hidecallergraph
 */
UPNPLIB_API const char* UpnpString_get_String(
    /*! [in] The \em \b this pointer. */
    const UpnpString* p);

/*!
 * \brief Sets the string from a pointer to char.
 * \hidecallergraph
 */
UPNPLIB_API int UpnpString_set_String(
    /*! [in] The \em \b this pointer. */
    UpnpString* p,
    /*! [in] (char *) to copy from. */
    const char* s);

/*!
 * \brief Sets the string from a pointer to char using a maximum of N chars.
 */
UPNPLIB_API int UpnpString_set_StringN(
    /*! [in] The \em \b this pointer. */
    UpnpString* p,
    /*! [in] (char *) to copy from. */
    const char* s,
    /*! Maximum number of chars to copy.*/
    size_t n);

/*!
 * \brief Clears the string, sets its size to zero.
 */
UPNPLIB_API void UpnpString_clear(
    /*! [in] The \em \b this pointer. */
    UpnpString* p);

/*!
 * \brief Compares two strings for equality. Case matters.
 *
 * \return The result of strcmp().
 */
UPNPLIB_API int UpnpString_cmp(
    /*! [in] The \em \b the first string. */
    UpnpString* p,
    /*! [in] The \em \b the second string. */
    UpnpString* q);

/*!
 * \brief Compares two strings for equality. Case does not matter.
 *
 * \return The result of strcasecmp().
 */
UPNPLIB_API int UpnpString_casecmp(
    /*! [in] The \em \b the first string. */
    UpnpString* p,
    /*! [in] The \em \b the second string. */
    UpnpString* q);

/// @} UpnpString The UpnpString API

#endif // COMPA_UPNPSTRING_HPP
