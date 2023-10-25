#ifndef UPNPLIB_NET_SOCKADDR_HPP
#define UPNPLIB_NET_SOCKADDR_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-25

// Helpful links:
// clang-format off
// REF: [sockaddr structures as union](https://stackoverflow.com/a/76548581/5014688)
// REF: [Why do I get wrong pointer to a base class with a virtual constructor](https://stackoverflow.com/q/76360179/5014688)
// clang-format on

#include <upnplib/port_sock.hpp>
#include <upnplib/visibility.hpp>
#include <string>

namespace upnplib {

// Free function to get the port number from a string
// --------------------------------------------------
UPNPLIB_API uint16_t to_port(const std::string& a_port_str);

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
to_addrport_str(const ::sockaddr_storage* const a_sockaddr);

// Free function to logical compare two sockaddr structures
// --------------------------------------------------------
// To have a logical equal socket address we compare the address family, the ip
// address and the port.
UPNPLIB_API bool sockaddrcmp(const ::sockaddr_storage* a_ss1,
                             const ::sockaddr_storage* a_ss2);


// Specialized sockaddr structure
// ==============================
struct UPNPLIB_API SSockaddr_storage {
    ::sockaddr_storage ss{};

    // Constructor
    SSockaddr_storage();

    // Destructor
    virtual ~SSockaddr_storage();

    // Get reference to the sockaddr_storage structure.
    // Only as example, I don't use it because it may be confusing. I only use
    // SSockaddr_storage::ss (instantiated e.g. ssObj.ss) to access the trivial
    // member structure.
    // operator const ::sockaddr_storage&() const;

    // Assignment operator= to set socket address from string,
    // e.g.: SSockaddr_storage ss; ss = "[2001:db8::1]";
    // Input examples: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                 "192.168.1.1", "192.168.1.1:50001".
    // An empty address string clears the address storage.
    void operator=(const std::string& a_addr_str);

    // Compare operator== to test if another socket address is equal to this.
    // It only supports AF_INET6 and AF_INET. For all other address families it
    // returns false.
    bool operator==(const ::sockaddr_storage&) const;

    // Getter for the assosiated ip address without port, e.g.
    // "[2001:db8::2]" or "192.168.254.253".
    std::string get_addr_str() const;

    // Getter for the numeric port.
    uint16_t get_port() const;

    // Getter for the length of the sockaddr structure.
    socklen_t get_sslen() const;

  private:
    UPNPLIB_LOCAL void handle_ipv6(const std::string& a_addr_str);
    UPNPLIB_LOCAL void handle_ipv4(const std::string& a_addr_str);
    UPNPLIB_LOCAL void handle_port(const std::string& a_port);
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCKADDR_HPP
