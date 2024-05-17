#ifndef UPNPLIB_INCLUDE_ADDRINFO_HPP
#define UPNPLIB_INCLUDE_ADDRINFO_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-20
/*!
 * \file
 * \brief Declaration of the Addrinfo class.
 */

#include <upnplib/sockaddr.hpp>

namespace upnplib {

/*!
 * \brief Get information from the operating system about a
 * [netaddress](\ref glossary_netaddr)
<!-- ====================================================== -->
 * \ingroup upnplib-addrmodul
 *
 * This is a stripped version. It is only used in a snapshot to get information
 * about a netaddress. There is no need to copy the object. The last full
 * featured version with copy constructor, copy asignment operator, compare
 * operator, additional getter and its unit tests can be found at
 * Github commit e2ffc0c46a2d8f15390f2816e1a18782e500fd09
 * */
class UPNPLIB_API CAddrinfo {
  public:
    /*! \brief Constructor for getting an address information with port number
     * string. */
    CAddrinfo(std::string_view a_node, std::string_view a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    /*! \brief Constructor for getting an address information with numeric port
     * number. */
    CAddrinfo(std::string_view a_node, in_port_t a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    /*! \brief Constructor for getting an address information from only a
     * netaddress */
    CAddrinfo(std::string_view a_node);


    /*! \brief Destructor */
    virtual ~CAddrinfo();

    /// \cond
    // Copy constructor
    // We cannot use the default copy constructor because there is also
    // allocated memory for the addrinfo structure to copy. We get segfaults
    // and program aborts. This class is not used to copy the object.
    CAddrinfo(const CAddrinfo&) = delete;

    // Copy assignment operator
    // Same as with the copy constructor.
    CAddrinfo& operator=(CAddrinfo) = delete;
    /// \endcond

    /*! \name Setter
     * *************
     * @{ */
    /*! \brief Initialize address information that is returned by syscall
     * ::%getaddrinfo()
     *
     * \code
// Usage e.g.:
CAddrinfo ai("[2001:db8::1]", 50050, AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
try {
    ai.init();
} catch (const std::runtime_error& e) { handle_failed_address_info();
} catch (const std::invalid_argument& e) { handle_other_error();
}
normal_execution();
     * \endcode
     * \note It is important to careful check the error situation because this
     * initialization depends on the real environment over which we have no
     * influence. Name resolution may fail because to be unspecified, DNS
     * server may be temporary down, etc.
     *
     * Usually this setter is called one time after constructing the object.
     * This gets an address information from the operating system that may also
     * use its internal name resolver inclusive contacting external DNS server.
     * If you use the flag **AI_NUMERICHOST** with the constructor then a
     * possible expensive name resolution to DNS server is suppressed.
     *
     * Because always the same cached hints given with the constructor are used
     * we also get the same information but with new allocated memory. So it
     * doesn't make much sense to call it more than one time, but doesn't hurt
     * except waste of resources.
     *
     * \exception std::runtime_error Failed to get address information, node or
     * service not known. Maybe an alphanumeric node name that cannot be
     * resolved. Or the DNS server is temporary not available.
     * \exception std::invalid_argument Other system error, e.g. address family
     * or socket type not supported, etc.
     *
     * Provides strong exception guarantee. It should be noted that are
     * different error messages returned by different platforms.
     */
    void init();
    /// @} Setter


    /*! \name Getter
     * *************
     * @{ */
    /*! \brief Read access to members of the <a
     * href="https://www.man7.org/linux/man-pages/man3/getaddrinfo.3.html#DESCRIPTION">addrinfo
     * structure</a>
     * \code
     * // Usage e.g.:
     * CAddrinfo ai("localhost", 50001, AF_UNSPEC, SOCK_STREAM);
     * try {
     *     ai.init();
     * } catch (xcp) { handle_error(); }
     * if (ai->ai_socktype == SOCK_DGRAM) {} // is SOCK_STREAM in this example
     * if (ai->ai_family == AF_INET6) { do_this(); };
     * \endcode
     *
     * The operating system returns the information in a structure that you can
     * read to get all details. */
    // REF:_<a_href="https://stackoverflow.com/a/8782794/5014688">Overloading_member_access_operators_->,_.*</a>
    ::addrinfo* operator->() const noexcept;

    /*! \brief Get the assosiated [netaddress](\ref glossary_netaddr) with port
     * \code
     * // Usage e.g.:
     * CAddrinfo ai("localhost", 50001, AF_UNSPEC, SOCK_STREAM);
     * try {
     *     ai.init();
     * } catch (xcp) { handle_error(); }
     * std::string netaddrp = ai.netaddrp();
     * if (netaddrp == "[::1]:50001") { manage_ipv6_interface();
     * } else if (netaddrp == "127.0.0.1:50001") { manage_ipv4_interface(); }
     * \endcode */
    virtual const std::string netaddrp() const noexcept;
    /// @} Getter

  private:
    // Cache the hints that are given with the constructor by the user, so we
    // can always get identical address information from the operating system.
    DISABLE_MSVC_WARN_4251
    std::string m_node;
    std::string m_service;
    ENABLE_MSVC_WARN
    addrinfo m_hints{};

    // Pointer to the address information returned from systemcall
    // ::getaddrinfo(). This pointer must be freed. That is done with the
    // destructor. It is initialized to point to the hints so there is never a
    // dangling pointer that segfaults. Pointing to the hints means there is no
    // information available.
    addrinfo* m_res{&m_hints};

    // Private method to free allocated memory for address information.
    void free_addrinfo() noexcept;
};

} // namespace upnplib

#endif // UPNPLIB_INCLUDE_ADDRINFO_HPP
