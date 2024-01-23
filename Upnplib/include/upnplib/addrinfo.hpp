#ifndef UPNPLIB_INCLUDE_ADDRINFO_HPP
#define UPNPLIB_INCLUDE_ADDRINFO_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-20

#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <upnplib/port_sock.hpp>
#include <upnplib/sockaddr.hpp>

#include <string>

namespace upnplib {

// Provide C style addrinfo as class and wrap its system calls
// ===========================================================
// Typically, a complete socket address consists of IP address, port and socket
// type. For example '[2001:db8::1]:8080, type SOCK_STREAM' is different from
// '[2001:db8::1]:8080, type SOCK_DGRAM' or from '[2001:db8::1]:50000, type
// SOCK_STREAM'. Binding will not complain "address already in use".

class UPNPLIB_API CAddrinfo : public SSockaddr {
  public:
    // Constructor for getting an address information with port number string.
    CAddrinfo(const std::string& a_node, const std::string& a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    // Constructor for getting an address information with numeric port number.
    CAddrinfo(const std::string& a_node, in_port_t a_service,
              const int a_family = AF_UNSPEC, const int a_socktype = 0,
              const int a_flags = 0, const int protocol = 0);

    // Rule of three: we need a copy constructor and a copy assignment operator.
    // REF: [What is The Rule of Three?]
    // (https://stackoverflow.com/q/4172722/5014688)
    //
    // copy constructor:
    // Example: CAddrinfo ai2 = ai1; // ai1 is an instantiated valid object,
    // or       CAddrinfo ai2{ai1};
    CAddrinfo(const CAddrinfo& that);
    //
    // copy assignment operator:
    // Provides strong exception guarantee.
    // Example: ai2 = ai1; // ai1 and ai2 are instantiated valid objects.
    CAddrinfo& operator=(CAddrinfo that);

    // Destructor
    virtual ~CAddrinfo();

    // Compare operator== to test if another address info is equal to this.
    // It only supports AF_INET6 and AF_INET. For all other address families it
    // returns false.
    bool operator==(const CAddrinfo&) const;

    // This is to have read access to members of the addrinfo structure,
    // Example: CAddrinfo ai(..); if(ai->ai_family == AF_INET6) {..};
    // REF: [Overloading member access operators ->, .*]
    // https://stackoverflow.com/a/8782794/5014688
    ::addrinfo* operator->() const;

  private:
    // This pointer is the reason why we need a copy constructor and a copy
    // assignment operator.
    addrinfo* m_res{nullptr};

    // Cache the hints that are given by the user, so we can always get
    // identical address information from the operating system.
    DISABLE_MSVC_WARN_4251
    std::string m_node;
    std::string m_service;
    std::string m_netaddr; // For a netaddress without port
    ENABLE_MSVC_WARN
    mutable addrinfo m_hints{};

    // This is a helper method that gets a new address information from the
    // operating system. Because we always use the same cached hints we also get
    // the same information with new allocated memory. We cannot just copy the
    // structure that m_res pointed to (*m_res = *that.m_res;). Copy works but
    // MS Windows failed to destruct it with freeaddrinfo(m_res);. It throws an
    // exception "A non-recoverable error occurred during a database lookup.".
    // Seems there are also pointer within the addrinfo structure that are not
    // deeply copied. So we have to go the hard way with getaddrinfo() and free
    // it with freeaddrinfo(),
    // Provides strong exception guarantee.
    UPNPLIB_LOCAL addrinfo* get_addrinfo() const;
};

} // namespace upnplib

#endif // UPNPLIB_INCLUDE_ADDRINFO_HPP
