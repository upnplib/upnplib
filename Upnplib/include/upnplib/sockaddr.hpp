#ifndef UPNPLIB_NET_SOCKADDR_HPP
#define UPNPLIB_NET_SOCKADDR_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-09
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

namespace upnplib {

// Never need to use type casts with pointer to different socket address
// structures. For details about using this helpful union have a look at
// REF:_[sockaddr_structures_as_union](https://stackoverflow.com/a/76548581/5014688)
union sockaddr_t {
    ::sockaddr_storage ss;
    ::sockaddr_in6 sin6;
    ::sockaddr_in sin;
    ::sockaddr sa;
};

// Free function to get the address string from a sockaddr structure
// -----------------------------------------------------------------
// Throws exception 'invalid argument' with unsupported address family.
// Supported is only AF_INET6 and AF_INET.
UPNPLIB_API std::string to_addr_str(const ::sockaddr_storage* const a_sockaddr);

// Free function to get the address string with port from a sockaddr structure
// ---------------------------------------------------------------------------
// Throws exception 'invalid argument' with unsupported address family.
// Supported is only AF_INET6 and AF_INET.
UPNPLIB_API std::string
to_addrp_str(const ::sockaddr_storage* const a_sockaddr);

// Free function to get the port number from a string
// --------------------------------------------------
// Throws exception 'invalid argument' with invalid port number.
UPNPLIB_API uint16_t to_port(const std::string& a_port_str);

// Free function to logical compare two sockaddr structures
// --------------------------------------------------------
// To have a logical equal socket address we compare the address family, the ip
// address and the port.
UPNPLIB_API bool sockaddrcmp(const ::sockaddr_storage* a_ss1,
                             const ::sockaddr_storage* a_ss2) noexcept;


// Specialized sockaddr structure
// ==============================
struct UPNPLIB_API SSockaddr {
    // References to have direct access to the trival structures in the private
    // union.
    sockaddr_storage& ss = m_sa_union.ss;
    sockaddr_in6& sin6 = m_sa_union.sin6;
    sockaddr_in& sin = m_sa_union.sin;
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

    // copy constructor: needed for copy assignment operator.
    // Example: SSockaddr saddr2 = saddr1; // saddr1 is an instantiated object,
    // or       SSockaddr saddr2{saddr1};
    SSockaddr(const SSockaddr&);
    //
    // copy assignment operator, needs user defined copy contructor.
    // Provides strong exception guarantee with value argument.
    // Example: saddr2 = saddr1; // saddr? are instantiated valid objects.
    SSockaddr& operator=(SSockaddr); // value argument

    // Assignment operator= to set socket address from string,
    // e.g.: SSockaddr ss; ss = "[2001:db8::1]";
    // Input examples: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                 "192.168.1.1", "192.168.1.1:50001".
    // An empty netaddress "" clears the address storage.
    // An invalid netaddress throws an exception 'std::invalid_argument'.
    void operator=(const std::string& a_addr_str);
    // To set the port number from an integer.
    void operator=(const in_port_t a_port);

    // Compare operator== to test if another socket address is equal to this.
    // It only supports AF_INET6 and AF_INET. For all other address families it
    // returns false.
    bool operator==(const ::sockaddr_storage&) const;

    // Getter for the assosiated ip address without port, e.g.
    // "[2001:db8::2]" or "192.168.254.253".
    virtual const std::string& get_addr_str();

    // Getter for the assosiated ip address with port, e.g.
    // "[2001:db8::2]:50001" or "192.168.254.253:50001".
    virtual const std::string& get_addrp_str();

    // Getter for the numeric port.
    virtual in_port_t get_port() const;

    // Getter for sizeof the Sockaddr Structure.
    socklen_t sizeof_ss() const;

    // Getter for sizeof the current (sin6 or sin) Sockaddr Structure.
    socklen_t sizeof_saddr() const;

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
/// \endcond

#endif // UPNPLIB_NET_SOCKADDR_HPP
