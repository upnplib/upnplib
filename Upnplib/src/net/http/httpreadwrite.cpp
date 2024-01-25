// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-19
/*!
 * \file
 * \brief Definition class Uri
 */

#include <upnplib/httpreadwrite.hpp>
#include <stdexcept>

namespace upnplib {

CUri::CUri(std::string a_url_str) : url_str(a_url_str) {
    // Exception: no
    const auto start = this->url_str.find("://");
    if (start == std::string::npos)
        throw std::invalid_argument(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ + ". '://' not found in url."));

    // Exception: no
    const auto end = this->url_str.find_first_of("/", start + 3);
    if (end == std::string::npos)
        throw std::invalid_argument(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", constructor " + __func__ +
                        ". hostport delimiter '/' not found in url."));

    const auto hostport_size = end - start - 3;
    if (hostport_size == 0)
        throw std::invalid_argument(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ + ". 'No hostport found in url."));

    // Exception: std::out_of_range if pos > size()
    this->hostport = this->url_str.substr(start + 3, hostport_size);
}

} // namespace upnplib
