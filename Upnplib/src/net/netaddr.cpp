// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-07-03
/*!
 * \file
 * \brief Definition of the Netaddr class.
 */

#include <upnplib/netaddr.hpp>

#include <upnplib/global.hpp>
#include <upnplib/addrinfo.hpp>
#include <upnplib/synclog.hpp>
#include <umock/netdb.hpp>


namespace upnplib {

// Free function to check for a netaddress without port
// ----------------------------------------------------
// I use the system function ::getaddrinfo() to check if the node string is
// acceptable. Using ::getaddrinfo() is needed to cover all special address
// features like scope id for link local addresses, Internationalized Domain
// Names, and so on.
sa_family_t is_netaddr(const std::string& a_node,
                       const int a_addr_family) noexcept {
    // clang-format off
    TRACE("Executing is_netaddr(\"" + a_node + "\", " +
          (a_addr_family == AF_INET6 ? "AF_INET6" :
          (a_addr_family == AF_INET ? "AF_INET" :
          (a_addr_family == AF_UNSPEC ? "AF_UNSPEC" :
          std::to_string(a_addr_family)))) + ")")
    // clang-format on

    // The shortest numeric netaddress string is "[::]".
    if (a_node.size() < 4) { // noexcept
        return AF_UNSPEC;
    }

    // Provide resources for ::getaddrinfo()
    // AI_NUMERICHOST ensures that only numeric addresses accepted.
    std::string node;
    ::addrinfo hints{};
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = a_addr_family;
    ::addrinfo* res{nullptr};

    // Check for ipv6 addresses and remove surounding brackets for
    // ::getaddrinfo().
    // front() and back() have undefined behavior with an empty string. Here
    // its size() is at least 4. Substr() throws exception out of range if pos
    // > size(). All this means that we cannot get an exception here.
    if (a_node.front() == '[' && a_node.back() == ']' &&
        (a_addr_family == AF_UNSPEC || a_addr_family == AF_INET6)) {
        node = a_node.substr(1, a_node.length() - 2);
        hints.ai_family = AF_INET6;

    } else if (a_node.find_first_of(":") != std::string::npos) {
        // Ipv6 addresses are already checked and here are only ipv4 addresses
        // and URL names possible. Both are not valid if they contain a colon.
        // find_first_of() does not throw an exception.
        return AF_UNSPEC;

    } else {
        node = a_node;
    }

    // Call ::getaddrinfo() to check the remaining node string.
    int rc = umock::netdb_h.getaddrinfo(node.c_str(), nullptr, &hints, &res);
    TRACE2("syscall ::getaddrinfo() with res = ", res)
    if (rc != 0) {
        TRACE2("syscall ::freeaddrinfo() with res = ", res)
        umock::netdb_h.freeaddrinfo(res);
        UPNPLIB_LOGINFO "MSG1116: syscall ::getaddrinfo(\""
            << node.c_str() << "\", nullptr, " << &hints << ", " << &res
            << "), (" << rc << ") " << gai_strerror(rc) << '\n';
        return AF_UNSPEC;
    }

    int af_family = res->ai_family;
    TRACE2("syscall ::freeaddrinfo() with res = ", res)
    umock::netdb_h.freeaddrinfo(res);
    // Guard different types on different platforms (win32); need to cast to
    // af_family (unsigned short).
    if (af_family < 0 || af_family > 65535) {
        return AF_UNSPEC;
    }
    return static_cast<sa_family_t>(af_family);
}


// Free function to check if a string is a valid port number
// ---------------------------------------------------------
int is_numport(const std::string& a_port_str) noexcept {
    TRACE("Executing is_numport() with port=\"" + a_port_str + "\"")

    // Only non empty strings. I have to check this to avoid exception.
    if (a_port_str.empty())
        return -1;

    // Now we check if the string are all digit characters
    for (char ch : a_port_str) {
        if (!std::isdigit(static_cast<unsigned char>(ch)))
            return -1;
    }

    // Only strings with max. 5 char may be valid (uint16_t has max. 65535)
    if (a_port_str.length() > 5)
        return 1;

    // Valid positive number but is it within the port range (uint16_t)?
    // stoi may throw std::invalid_argument if no conversion could be
    // performed or std::out_of_range. But with the prechecked number string
    // this should never be thrown.
    return (std::stoi(a_port_str) <= 65535) ? 0 : 1;
}


// Netaddr class
// =============
Netaddr::Netaddr(){
    TRACE2(this, " Construct default Netaddr()") //
}

Netaddr::~Netaddr() {
    TRACE2(this, " Destruct Netaddr()") //
}

#if false
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
        if (is_netaddr(netaddr) == AF_UNSPEC) {
            // Unspecified netaddress
            return;
        }
        std::string port = a_addr_str.substr(pos + 1);
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
#endif

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
