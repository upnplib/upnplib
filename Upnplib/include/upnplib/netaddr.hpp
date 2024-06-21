#ifndef UPNPLIB_NET_NETADDR_HPP
#define UPNPLIB_NET_NETADDR_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-26
/*!
 * \file
 * \brief Declaration of the Netaddr class
 */

#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/socket.hpp>
#include <string>

namespace upnplib {

// Free function to check for a netaddress without port
// ----------------------------------------------------
/*! \brief Check for a [netaddress](\ref glossary_netaddr) and return its
 * address family
 * \ingroup upnplib-addrmodul
 * \code
 * // Usage e.g.:
 * if (is_netaddr("[2001:db8::1]") != AF_UNSPEC) { manage_given_netaddress(); }
 * if (is_netaddr("[2001:db8::1]", AF_INET) == AF_INET) { // nothing to do }
 * if (is_netaddr("[fe80::1%2]") == AF_INET6) { manage_link_local_addr(); }
 * \endcode
 *
 * Checks if a string is a netaddress without port and returns its address
 * family.
 *
 * \returns
 *  On success: Address family AF_INET6 or AF_INET the address belongs to\n
 *  On error: AF_UNSPEC, the address is alphanumeric (maybe a DNS name?)
 */
sa_family_t is_netaddr(
    /// [in] string to check for a netaddress.
    const std::string& a_node,
    /// [in] optional: AF_INET6 or AF_INET to preset the address family to look
    /// for.
    const int a_addr_family = AF_UNSPEC) noexcept;


// Free function to check if a string is a valid port number
// ---------------------------------------------------------
/*! \brief Check if a given string represents a port number
 * \ingroup upnplib-addrmodul
 * \code
 * // Usage e.g.:
 * if (is_numport("55555") == 0) { manage_given_port(); }
 * \endcode
 *
 * Checks if the given string represents a numeric port number between 0 and
 * 65535.
 *
 * \returns
 *  - **-1** if *a_port_str* isn't a numeric number, but it may be a valid
 *           service name (e.g. "https")
 *  - **0** if *a_port_str* is a valid numeric port number between 0 and 65535
 *  - **1** if *a_port_str* is an invalid numeric port number > 65535
 */
int is_numport(const std::string& a_port_str) noexcept;


// Netaddress class
// ================
/*! \brief Netaddress class to handle [netaddress](\ref glossary_netaddr)es
 * \ingroup upnplib-addrmodul
 */
class UPNPLIB_API Netaddr {
  public:
    // Default Constructor
    Netaddr();

    // Destructor
    virtual ~Netaddr();

#if 0
    /*! \name Setter
     * *************
     * @{ */
    // Assignment operator to set a netaddress
    // ---------------------------------------
    /*! \brief Assign a [netaddress](\ref glossary_netaddr)
     * \code
     * // Usage e.g.:
     * Netaddr netaddr;
     * netaddr = "[2001:db8::1]:56789";
     * std::string na_str = netaddr.str();
     * std::cout << netaddr << "\n"; // output "[2001:db8::1]:56789"
     * \endcode
     *
     * Assign rules are:\n
     * a netaddress consists of two parts, ip address and port. A netaddress
     * has always a port. With an invalid ip address the whole netaddress is
     * unspecified and results to "". Valid special cases are these well
     * defined unspecified addresses:
\verbatim
"[::]"          results to  "[::]:0"
"[::]:"         results to  "[::]:0"
"[::]:0"        results to  "[::]:0"
"[::]:65535"    results to  "[::]:65535" // port 0 to 65535
"0.0.0.0"       results to  "0.0.0.0:0"
"0.0.0.0:"      results to  "0.0.0.0:0"
"0.0.0.0:0"     results to  "0.0.0.0:0"
"0.0.0.0:65535" results to  "0.0.0.0:65535" // port 0 to 65535
\endverbatim
     * A valid address with an invalid port results to port 0, for example\n
\verbatim
"[2001:db8::51]:98765" results to "[2001:db8::51]:0"
\endverbatim
     */
    void operator=(
        /// [in] String with a possible netaddress
        const std::string& a_addr_str) noexcept;
    /// @} Setter
#endif

    /*! \name Getter
     * *************
     * @{ */
    // Getter for a netaddress string
    // ------------------------------
    /*! \brief Get [netaddress](\ref glossary_netaddr) string
     * \code
     * // Usage e.g.:
     * Netaddr netaddr;
     * netaddr = "[2001:db8::1]:56789";
     * std::string na_str = netaddr.str();
     * \endcode */
    std::string& str();
    /// @} Getter

  private:
    friend class CAddrinfo;

    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    std::string m_netaddrp;
};

/// \cond
// Getter of the netaddress to output stream
// -----------------------------------------
/*! \brief output the [netaddress](\ref glossary_netaddr)
 * \ingroup upnplib-addrmodul
 * \code
 * // Usage e.g.:
 * Netaddr netaddr;
 * netaddr = "[2001:db8::1]:56789";
 * std::cout << netaddr << "\n"; // output "[2001:db8::1]:56789"
 * \endcode
 */
UPNPLIB_API ::std::ostream& operator<<(::std::ostream& os, Netaddr& nap);
/// \endcond

} // namespace upnplib

#endif // UPNPLIB_NET_NETADDR_HPP
