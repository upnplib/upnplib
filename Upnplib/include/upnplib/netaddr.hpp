#ifndef UPNPLIB_NET_NETADDR_HPP
#define UPNPLIB_NET_NETADDR_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-23
/*!
 * \file
 * \brief Declaration of the Netaddr class
 */

#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
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
 * if (is_port("55555") { manage_given_port(); }
 * \endcode
 *
 * Checks if the given string represents a numeric value between 0 and 65535.
 * That also means that an empty port string "" returns false.
 *
 * \returns
 *   **true** if string represents a port number 0..65535\n
 *   **false** otherwise, inclusive if having an empty port string ""
 */
bool is_port(const std::string& a_port_str) noexcept;


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
     */
    void operator=(
        /// [in] String with a possible netaddress
        const std::string& a_addr_str) noexcept;
    /// @} Setter


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
