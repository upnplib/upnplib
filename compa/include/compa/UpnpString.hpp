#ifndef COMPA_UPNPSTRING_HPP
#define COMPA_UPNPSTRING_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-26

#include "upnplib/visibility.hpp"
#include <stddef.h> // For size_t

namespace compa {

struct UpnpString {
    /*! \brief Length of the string excluding terminating null byte ('\0'). */
    size_t m_length;
    /*! \brief Pointer to a dynamically allocated area that holds the NULL
     * terminated string. */
    char* m_string;
};

/*!
 * \return A pointer to a new allocated object.
 */
UPNPLIB_API UpnpString* UpnpString_new(void);

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
// EXPORT_SPEC UpnpString *UpnpString_dup(
//        /*! [in] The \em \b this pointer. */
//        const UpnpString *p);

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

} // namespace compa

/* @} UpnpString The UpnpString API */

#endif // COMPA_UPNPSTRING_HPP
