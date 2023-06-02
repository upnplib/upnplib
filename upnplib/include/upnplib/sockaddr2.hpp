#ifndef UPNPLIB_NET_SOCKADDR2_HPP
#define UPNPLIB_NET_SOCKADDR2_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-04

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <upnplib/port_sock.hpp>
#include <upnplib/visibility.hpp>
#include <upnplib/port.hpp>
#include <string>

namespace upnplib {

namespace rework {

// Free function to get the port number from a string
// --------------------------------------------------
UPNPLIB_API uint16_t to_port(const std::string& a_port_str);

// Free function to get the address string from a sockaddr structure
// -----------------------------------------------------------------
UPNPLIB_API std::string to_addr_str(const ::sockaddr_storage* const a_sockaddr);

} // namespace rework


// Ssockaddr_storage structure derived from the C structure ::sockaddr_storage
// ===========================================================================
struct UPNPLIB_API Ssockaddr_storage : public ::sockaddr_storage {
    // Constructor
    Ssockaddr_storage();

    // Destructor
    virtual ~Ssockaddr_storage();

    // Assignment operator= to set socket address from string,
    // e.g.: Ssockaddr_storage ss; ss = "[2001:db8::1]";
    // Input examples: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                 "192.168.1.1", "192.168.1.1:50001".
    // An empty address string clears the address storage.
    void operator=(const std::string& a_addr_str);

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

#endif // UPNPLIB_NET_SOCKADDR2_HPP
