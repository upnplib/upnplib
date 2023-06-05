#ifndef UPNPLIB_NET_SOCKADDR_HPP
#define UPNPLIB_NET_SOCKADDR_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-05

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <upnplib/port_sock.hpp>
#include <upnplib/visibility.hpp>
#include <string>

namespace upnplib {

// Free function to get the port number from a string
// --------------------------------------------------
UPNPLIB_API uint16_t to_port(const std::string& a_port_str);

// Free function to get the address string from a sockaddr structure
// -----------------------------------------------------------------
UPNPLIB_API std::string to_addr_str(const ::sockaddr_storage* const a_sockaddr);

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
    // Only as example, we don't use it.
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


  private:
    UPNPLIB_LOCAL void handle_ipv6(const std::string& a_addr_str);
    UPNPLIB_LOCAL void handle_ipv4(const std::string& a_addr_str);
    UPNPLIB_LOCAL void handle_port(const std::string& a_port);
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCKADDR_HPP
