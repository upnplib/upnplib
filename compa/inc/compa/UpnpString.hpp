#ifndef COMPA_UPNPSTRING_HPP
#define COMPA_UPNPSTRING_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-08

#include "upnplib/visibility.hpp"
#include <stddef.h> // For size_t

typedef struct s_UpnpString UpnpString;

namespace compa {

/*!
 * \brief Returns the length of the string.
 *
 * \return The length of the string.
 * */
UPNPLIB_API size_t UpnpString_get_Length(
    /*! [in] The \em \b this pointer. */
    const UpnpString* p);

/*!
 * \brief Returns the pointer to char.
 *
 * \return The pointer to char.
 */
UPNPLIB_API const char* UpnpString_get_String(
    /*! [in] The \em \b this pointer. */
    const UpnpString* p);

/*!
 * \brief Sets the string from a pointer to char.
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

} // namespace compa

#endif // COMPA_UPNPSTRING_HPP
