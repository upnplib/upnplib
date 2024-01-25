#ifndef UPNPLIB_UPNPTOOLS_HPP
#define UPNPLIB_UPNPTOOLS_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-25
/*!
 * \file
 * \brief General usable free function tools and helper.
 */

#include <upnplib/visibility.hpp> // for UPNPLIB_API
#include <string>

namespace upnplib {

/*! \brief Get error name string.
 * \return Name string of the error */
UPNPLIB_API const std::string errStr( //
    int error ///< [in] Error number
);

/*! \brief Get extended error name string.
 * \return Error message with hint what should be correct */
UPNPLIB_API const std::string errStrEx( //
    const int error, /*!< [in] Error number */
    const int success /*!< [in] Message number that should be given instead of
                         the error */
);

} // namespace upnplib

#endif // UPNPLIB_UPNPTOOLS_HPP
