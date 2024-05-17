#ifndef UPNPLIB_NET_NETADDR_HPP
#define UPNPLIB_NET_NETADDR_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-20
/*!
 * \file
 * \brief Declaration of the Netaddr class
 */

#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <string>

namespace upnplib {

/*!
 * \brief Netaddress class to handle [netaddress](\ref glossary_netaddr)es
 * \ingroup upnplib-addrmodul
 */
class UPNPLIB_API CNetaddr {
  public:
    /// Default Constructor
    CNetaddr();

    /// Destructor
    ~CNetaddr();

    /*! \name Setter
     * *************
     * @{ */
    /// Set a netaddress
    void set(std::string_view a_node, std::string_view a_service);
    /// @} Setter

  private:
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    std::string m_netaddrp;
};

} // namespace upnplib

#endif // UPNPLIB_NET_NETADDR_HPP
