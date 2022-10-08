#ifndef UPNPLIB_COMPA_UPNPSTRING_HPP
#define UPNPLIB_COMPA_UPNPSTRING_HPP
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

} // namespace compa

#endif // UPNPLIB_COMPA_UPNPSTRING_HPP
