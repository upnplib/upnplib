#ifndef UPNPLIB_NET_SOCKADDR_HPP
#define UPNPLIB_NET_SOCKADDR_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-12
/*!
 * \file
 * \brief Declaration of the Sockaddr class and some free helper functions.
 */

// Helpful links:
// REF:_[Why_do_I_get_wrong_pointer_to_a_base_class_with_a_virtual_constructor](https://stackoverflow.com/q/76360179/5014688)

#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
/// \cond
#include <string>
/// \endcond

namespace upnplib {

/*! \brief type of a [netaddress](\ref glossary_netaddr)
 * \ingroup upnplib-addrmodul */
using netaddr_t = std::string;

/*!
 * \brief Helpful union of the different socket address structures
 * \ingroup upnplib-addrmodul
 *
 * Never need to use type casts with pointer to different socket address
 * structures. For details about using this helpful union have a look at
 * <!--REF:--> <a href="https://stackoverflow.com/a/76548581/5014688">sockaddr
 * structures as union</a>
 */
union sockaddr_t {
    ::sockaddr_storage ss;
    ::sockaddr_in6 sin6;
    ::sockaddr_in sin;
    ::sockaddr sa;
};


// Free function
/*! \brief Get the [netaddress](\ref glossary_netaddr) without port from a
 * sockaddr structure
 * <!-- -------------------------------------------------------------- -->
 * \ingroup upnplib-addrmodul
 * \code
 * ~$ // Usage e.g.:
 * ~$ ::sockaddr_storage saddr{};
 * ~$ std::cout << "netaddress is " << to_netaddr(&saddr) << "\n";
 * \endcode
 */
UPNPLIB_API netaddr_t
to_netaddr(const ::sockaddr_storage* const a_sockaddr) noexcept;


// Free function
/*! \brief Get the [netaddress](\ref glossary_netaddr) with port from a sockaddr
 * structure
 * <!-- -------------------------------------------------------------------- -->
 * \ingroup upnplib-addrmodul
 * \code
 * ~$ // Usage e.g.:
 * ~$ ::sockaddr_storage saddr{};
 * ~$ std::cout << "netaddress is " << to_netaddrp(&saddr) << "\n";
 * \endcode
 */
UPNPLIB_API netaddr_t
to_netaddrp(const ::sockaddr_storage* const a_sockaddr) noexcept;


// Free function
/*! \brief Get the port number from a string
 * <!-- ------------------------------- -->
 * \ingroup upnplib-addrmodul
 * \code
 * ~$ // Usage e.g.:
 * ~$ in_port_t port = to_port("55555");
 * \endcode
 *
 * Checks if the given string represents a numeric value between 0 and 65535.
 * \returns
 *  On success: Value of the port number
 *  <!-- On error: **0** -->
 * \exception std::invalid_argument Invalid port number
 */
UPNPLIB_API in_port_t to_port(const std::string& a_port_str);


// Free function
/*! \brief logical compare two sockaddr structures
 * <!-- ------------------------------------- -->
 * \ingroup upnplib-addrmodul
 *
 * To have a logical equal socket address we compare the address family, the ip
 * address and the port.
 *
 * \returns
 *  \b true if socket addresses are logical equal\n
 *  \b false otherwise
 */
UPNPLIB_API bool sockaddrcmp(const ::sockaddr_storage* a_ss1,
                             const ::sockaddr_storage* a_ss2) noexcept;


/*!
 * \brief Trivial ::%sockaddr structures enhanced with methods
 * <!--   ==================================================== -->
 * \ingroup upnplib-addrmodul
\code
~$ // Usage e.g.:
~$ ::sockaddr_storage saddr{};
~$ SSockaddr saObj;
~$ ::memcpy(&saObj.ss, &saddr, saObj.sizeof_ss());
~$ std::cout << "netaddress of saObj is " << saObj.get_netaddr() << "\n";
\endcode
 *
 * This structure should be usable on a low level like the trival C `struct
 * ::%sockaddr_storage` but provides additional methods to manage its data.
 */
struct UPNPLIB_API SSockaddr {
    /// Reference to sockaddr_storage struct
    sockaddr_storage& ss = m_sa_union.ss;
    /// Reference to sockaddr_in6 struct
    sockaddr_in6& sin6 = m_sa_union.sin6;
    /// Reference to sockaddr_in struct
    sockaddr_in& sin = m_sa_union.sin;
    /// Reference to sockaddr struct
    sockaddr& sa = m_sa_union.sa;

    // Constructor
    SSockaddr();

    // Destructor
    virtual ~SSockaddr();

    // Get reference to the sockaddr_storage structure.
    // Only as example, I don't use it because it may be confusing. I only use
    // SSockaddr::ss (instantiated e.g. ssObj.ss) to access the trivial
    // member structure.
    // operator const ::sockaddr_storage&() const;

    /*! \brief Copy constructor, also needed for copy assignment operator.
     * \code
     * ~$ // Usage e.g.:
     * ~$ SSockaddr saddr2 = saddr1; // saddr1 is an instantiated object.
     * ~$ // or
     * ~$ SSockaddr saddr2{saddr1};
     * \endcode */
    SSockaddr(const SSockaddr&);

    /*! \brief Copy assignment operator, needs user defined copy contructor
     * \code
     * ~$ // Usage e.g.:
     * ~$ saddr2 = saddr1; // saddr? are two instantiated valid objects.
     * \endcode */
    // Strong exception guarantee with value argument as given.
    SSockaddr& operator=(SSockaddr); // value argument

    /*! \name Setter
     * *************
     * @{ */
    // Assignment operator
    /*! \brief Set socket address from a [netaddress](\ref glossary_netaddr)
     * \code
     * ~$ // Usage e.g.:
     * ~$ SSockaddr saObj;
     * ~$ saObj = "[2001:db8::1]";
     * ~$ saObj = "[2001:db8::1]:50001";
     * ~$ saObj = "192.168.1.1";
     * ~$ saObj = "192.168.1.1:50001";
     *  \endcode
     * An empty netaddress "" clears the address storage.
     * \exception std::invalid_argument Invalid netaddress */
    void operator=(const std::string& a_addr_str);

    // Assignment operator
    /*! \brief Set [port number](\ref glossary_port) from integer
     * \code
     * ~$ // Usage e.g.:
     * ~$ SSockaddr saObj;
     * ~$ saObj = 50001;
     * \endcode */
    void operator=(const in_port_t a_port);
    /// @} Setter

    /*! \name Getter
     * *************
     * @{ */
    /*! \brief Test if another socket address is logical equal to this
     * \returns
     *  \b true if socket addresses are logical equal\n
     *  \b false otherwise
     *
     * It only supports AF_INET6 and AF_INET. For all other address families it
     * returns false. */
    bool operator==(const ::sockaddr_storage&) const;

    /*! \brief Get the assosiated [netaddress](\ref glossary_netaddr) without
     * port
     * \code
     * ~$ // Usage e.g.:
     * ~$ SSockaddr saObj;
     * ~$ if (saObj.get_netaddr() == "[::1]") { manage_localhost(); }
     * \endcode */
    virtual const netaddr_t& get_netaddr();

    /*! \brief Get the assosiated [netaddress](\ref glossary_netaddr) with port
     * \code
     * ~$ // Usage e.g.:
     * ~$ SSockaddr saObj;
     * ~$ if (saObj.get_netaddrp() == "[::1]:49494") { manage_localhost(); }
     * \endcode */
    virtual const netaddr_t& get_netaddrp();

    /// \brief Get the numeric port
    virtual in_port_t get_port() const;

    /// \brief Get sizeof the Sockaddr Structure
    socklen_t sizeof_ss() const;

    /// Get sizeof the current (sin6 or sin) Sockaddr Structure
    socklen_t sizeof_saddr() const;
    /// @} Getter

  private:
    sockaddr_t m_sa_union{}; // this is the union of trivial sockaddr structures
                             // that is managed.

    // Two buffer to have the strings valid for the lifetime of the object. This
    // is important for pointer to the string, for example with getting a C
    // string by using '.c_str()'.
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    std::string m_netaddr; // For a netaddress without port
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    std::string m_netaddrp; // For a netaddress with port

    UPNPLIB_LOCAL void handle_ipv6(const std::string& a_addr_str);
    UPNPLIB_LOCAL void handle_ipv4(const std::string& a_addr_str);
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCKADDR_HPP
