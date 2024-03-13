// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-14
/*!
 * \file
 * \brief Global used flag and emulated system function.
 */

#include <upnplib/global.ipp>
/// \cond

// strndup() is a GNU extension.
#ifndef HAVE_STRNDUP
char* strndup(const char* __string, size_t __n) {
    size_t strsize = strnlen(__string, __n);
    char* newstr = (char*)malloc(strsize + 1);
    if (newstr == NULL)
        return NULL;

    strncpy(newstr, __string, strsize);
    newstr[strsize] = 0;

    return newstr;
}
#endif

namespace upnplib {

UPNPLIB_API bool g_dbug{false};

} // namespace upnplib

/// \endcond
