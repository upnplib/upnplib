// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-23
/*!
 * \file
 * \brief Definition of the Netaddr class.
 */

#include <upnplib/netaddr.hpp>

#include <upnplib/global.hpp>
#include <upnplib/addrinfo.hpp>
#include <upnplib/synclog.hpp>


namespace upnplib {

// Free function to check for a netaddress without port
// ----------------------------------------------------
// I simply use the system function %inet_pton() to check if the node string is
// accepted.
sa_family_t is_netaddr(const std::string& a_node,
                       const int a_addr_family) noexcept {
    // clang-format off
    TRACE("Executing is_netaddr(\"" + a_node + "\", " +
          (a_addr_family == AF_INET6 ? "AF_INET6" :
          (a_addr_family == AF_INET ? "AF_INET" : "AF_UNSPEC")) + ")")
    // clang-format on

    // The shortest numeric netaddress is "[::]".
    if (a_node.size() < 4) // noexcept
        return AF_UNSPEC;

    unsigned char buf[sizeof(in6_addr)];
    switch (a_addr_family) {
    case AF_INET6: {
        // front() and back() have undefined behavior with an empty string.
        // Here its size() is at least 4.
        if (a_node.front() != '[' || a_node.back() != ']')
            return AF_UNSPEC;
        // Remove surounding brackets for inet_pton().
        // substr() throws exception std::out_of_range if pos > size(), but that
        // cannot occur due to the guard above, size() is at least 4 here and
        // pos is fix 1.
        const std::string node{a_node.substr(1, a_node.size() - 2)};
        return ::inet_pton(AF_INET6, node.c_str(), buf) == 1 ? AF_INET6
                                                             : AF_UNSPEC;
    }
    case AF_INET:
        return ::inet_pton(AF_INET, a_node.c_str(), buf) == 1 ? AF_INET
                                                              : AF_UNSPEC;
    case AF_UNSPEC:
        // Recursive call
        // clang-format off
        return is_netaddr(a_node, AF_INET6) ? AF_INET6 : false ||
               is_netaddr(a_node, AF_INET) ? AF_INET : AF_UNSPEC;
        // clang-format on
    }
    return AF_UNSPEC;
}


// Free function to check if a string is a valid port number
// ---------------------------------------------------------
bool is_port(const std::string& a_port_str) noexcept {
    TRACE("Executing is_port() with port=\"" + a_port_str + "\"")

    // Only non empty strings
    // and strings with max. 5 characters are valid (uint16_t has max. 65535)
    if (a_port_str.empty() || a_port_str.length() > 5)
        return false;

    // Now we check if the string are all digit characters
    for (char ch : a_port_str) {
        if (!::isdigit(static_cast<unsigned char>(ch)))
            return false;
    }
    // Valid positive number but is it within the port range (uint16_t)?
    // stoi may throw std::invalid_argument if no conversion could be performed
    // or std::out_of_range. But with the prechecked number string this should
    // never be thrown.
    return (std::stoi(a_port_str) <= 65535) ? true : false;
}


// Netaddr class
// =============
Netaddr::Netaddr(){
    TRACE2(this, " Construct default Netaddr()") //
}

Netaddr::~Netaddr() {
    TRACE2(this, " Destruct Netaddr()") //
}

// Assignment operator to set a netaddress
// ---------------------------------------
void Netaddr::operator=(const std::string& a_addr_str) noexcept {
    TRACE2(this, " Executing Netaddr=(), assign netaddress")

    m_netaddrp.clear(); // noexcept

    try {
        if (is_netaddr(a_addr_str) != AF_UNSPEC) { // noexcept
            // Here we have a valid netaddress without port.
            m_netaddrp = a_addr_str + ":0"; // maybe throw std::length_error?
            return;
        }
        // Following a valid netaddress must have a port.

        // Look for a port part or an ambiguous netaddress.
        size_t pos = a_addr_str.find_last_of(':'); // noexcept
        if (pos == std::string::npos || a_addr_str == "[::1") {
            // No port detected. The address string cannot be a netaddress.
            return;
        }

        // Split address string into possible netaddress and port and check
        // them. substr() throws exception std::out_of_range if pos > size().
        // pos is 0-based, size() is 1-based so with ":" is size()==1, pos==0,
        // pos+1==size() does not throw.
        std::string netaddr = a_addr_str.substr(0, pos);
        std::string port = a_addr_str.substr(pos + 1);
        if (is_netaddr(netaddr) == AF_UNSPEC) {
            // Unspecified netaddress
            netaddr.clear();
        }
        if (!is_port(port)) { // noexcept
            // Unspecified port
            port = "0";
        }

        // Set netaddress with port.
        if (!(netaddr.empty() && port == "0")) { // noexcept
            m_netaddrp = netaddr + ":" + port; // maybe throw std::length_error?
        }
        return;

    } catch (const std::length_error&) {
        // If a resulting string is too long then I just return with an
        // unspecified netaddress set.
        return;
    }
}

// Getter for a netaddress string
// ------------------------------
std::string& Netaddr::str() { return m_netaddrp; }

/// \cond
// Getter of the netaddress to output stream
// -----------------------------------------
std::ostream& operator<<(std::ostream& os, Netaddr& nap) {
    os << nap.str();
    return os;
}
/// \endcond

} // namespace upnplib
