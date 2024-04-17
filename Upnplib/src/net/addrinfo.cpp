// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-04-17
/*!
 * \file
 * \brief Definition of the Addrinfo class and free helper functions.
 */

#include <upnplib/addrinfo.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>
#include <umock/netdb.hpp>

#include <cstring>
/// \cond

namespace {

// Free function to test if the node is a valid numeric address string
// -------------------------------------------------------------------
// Checks if a string represents a numeric ipv6 or ipv4 address without port.
// On success it returns the socket address family AF_INET6 or AF_INET the
// address belongs to.
// Otherwise it returns 0.
// Note: I simply use the system function inet_pton() to check if the node
// string is accepted. This function can also be used to realize what address
// family an address belongs to. --Ingo
int is_numeric_node(const std::string& a_node,
                    const int a_addr_family = AF_UNSPEC) {
    // clang-format off
    TRACE("Executing is_numeric_node(\"" + a_node + "\", " +
          (a_addr_family == AF_INET6 ? "AF_INET6" :
          (a_addr_family == AF_INET ? "AF_INET" : "AF_UNSPEC")) + ")")
    // clang-format on
    if (a_node.empty())
        return 0;

    unsigned char buf[sizeof(in6_addr)];
    switch (a_addr_family) {
    case AF_INET6: {
        if (a_node.front() != '[' || a_node.back() != ']')
            return 0;
        // remove surounding brackets for inet_pton()
        const std::string node{a_node.substr(1, a_node.length() - 2)};
        return ::inet_pton(AF_INET6, node.c_str(), buf) ? AF_INET6 : 0;
    }
    case AF_INET:
        return ::inet_pton(AF_INET, a_node.c_str(), buf) ? AF_INET : 0;
    case AF_UNSPEC:
        // Recursive call
        // clang-format off
        return is_numeric_node(a_node, AF_INET6) ? AF_INET6 : 0 ||
               is_numeric_node(a_node, AF_INET) ? AF_INET : 0;
        // clang-format on
    }
    return 0;
}

} // namespace

namespace upnplib {

// CAddrinfo class to wrap ::addrinfo system calls
// ===============================================
// On Microsoft Windows this needs to have Windows Sockets initialized with
// WSAStartup().

// Constructor for getting an address information with port number string.
CAddrinfo::CAddrinfo(const std::string& a_node, const std::string& a_service,
                     const int a_family, const int a_socktype,
                     const int a_flags, const int a_protocol)
    : m_node(a_node), m_service(a_service),
      m_hints{a_flags, a_family, a_socktype, a_protocol,
              {},      nullptr,  nullptr,    nullptr} {
    TRACE2(this, " Construct CAddrinfo(..) with arguments")

    // Correct weak hints:
    // Always call is_numeric_node() with AF_UNSPEC to check all types. We
    // always set AI_NUMERICHOST if it match, no matter what 'a_family' was
    // reqested by the caller. But if AF_UNSPEC was requested, the found
    // address family must be set.
    int addr_family = is_numeric_node(m_node, AF_UNSPEC);
    if (addr_family) {
        m_hints.ai_flags |= AI_NUMERICHOST;
        if (m_hints.ai_family == AF_UNSPEC) {
            m_hints.ai_family = addr_family;
        }
    }

    // Get address information from the operating system.
    m_res = this->get_addrinfo(); // may throw exception
    // Copy socket address from the address info to this socket address:
    std::memcpy(&this->ss, m_res->ai_addr, m_res->ai_addrlen);
}


// Constructor for getting an address information with numeric port number.
// ------------------------------------------------------------------------
CAddrinfo::CAddrinfo(const std::string& a_node, in_port_t a_service,
                     const int a_family, const int a_socktype,
                     const int a_flags, const int a_protocol)
    : CAddrinfo(a_node, std::to_string(a_service), a_family, a_socktype,
                a_flags, a_protocol) {}


// Copy constructor
// ----------------
CAddrinfo::CAddrinfo(const CAddrinfo& that) : SSockaddr() {
    TRACE2(this, " Construct copy CAddrinfo()")
    m_node = that.m_node;
    m_service = that.m_service;
    m_hints = that.m_hints;
    m_res = this->get_addrinfo(); // may throw exception
    // Copy socket address from the address info to this socket address:
    std::memcpy(&this->ss, m_res->ai_addr, m_res->ai_addrlen);
}


// Copy assignment operator
// ------------------------
CAddrinfo& CAddrinfo::operator=(CAddrinfo that) {
    TRACE2(this, " Executing CAddrinfo::operator=()")
    // The argument by value ('that') was copied to the stack by the copy
    // constructor. It contains also a pointer (m_res) to a new allocated
    // addrinfo.
    std::swap(m_res, that.m_res);
    // The no longer needed current m_res pointer has been swapped to the stack
    // and its resource will be deallocated by the destructor of the 'that'
    // object when leaving this function.

    std::swap(m_node, that.m_node);
    std::swap(m_service, that.m_service);
    std::swap(m_hints, that.m_hints);
    std::swap(this->ss, that.ss);

    // by convention, always return *this
    return *this;
}


// Destructor
// ----------
CAddrinfo::~CAddrinfo() {
    TRACE2(this, " Destruct CAddrinfo()")
    TRACE2("syscall ::freeaddrinfo() with m_res = ", m_res)
    umock::netdb_h.freeaddrinfo(m_res);
    m_res = nullptr;
}


// Private method to get an addrinfo
// ---------------------------------
// Get address information with cached hints and ensure that we have a valid
// unknown address info for unknown nodes.
addrinfo* CAddrinfo::get_addrinfo() const {
    TRACE2(this, " Executing CAddrinfo::get_addrinfo)")

    // Check surounding brackets for AF_INET6 node address string.
    std::string node;
    if (!m_node.empty() && m_hints.ai_family == AF_INET6 &&
        (m_hints.ai_flags & AI_NUMERICHOST))
        // Here we have only ipv6 node strings representing a numeric ip
        // address.
        if (m_node.front() == '[' && m_node.back() == ']')
            // remove surounding brackets for ::getaddrinfo()
            node = m_node.substr(1, m_node.length() - 2);
        else
            // An ipv6 node string without enclosing brackets is never numeric
            // but we ask to be numeric. This is a failed condition for an
            // alphanumeric address string and we ask ::getaddrinfo() for an
            // unknown address info.
            node = "::";
    else
        node = m_node;

    // Very helpful for debugging to see what is given to ::getaddrinfo()
    // std::clog << "DEBUG: m_node = \"" << m_node << "\", node = \"" << node
    //           << "\", m_service = \"" << m_service
    //           << "\", m_hints.ai_flags = " << m_hints.ai_flags
    //           << ", m_hints.ai_family = " << m_hints.ai_family << "\n";

    // ::getaddrinfo()
    addrinfo* new_res{nullptr};
    int ret = umock::netdb_h.getaddrinfo(
        node.empty() ? nullptr : node.c_str(),
        m_service.empty() ? nullptr : m_service.c_str(), &m_hints, &new_res);

    if (ret == EAI_NONAME && new_res == nullptr) {
        // Node or service not known. This is a valid condition for example for
        // an alphanumeric node name that cannot be resolved by DNS. new_res
        // has been untouched by ::getaddrinfo(). We set an unknown address
        // information.
        m_hints.ai_flags |= AI_NUMERICHOST;
        if (m_hints.ai_family == AF_INET) {
            ret = umock::netdb_h.getaddrinfo(
                "0.0.0.0", m_service.empty() ? nullptr : m_service.c_str(),
                &m_hints, &new_res);
        } else {
            ret = umock::netdb_h.getaddrinfo(
                "::", m_service.empty() ? nullptr : m_service.c_str(), &m_hints,
                &new_res);
        }
    }
    if (ret != 0)
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1037: Failed to get address information: errid(" +
            std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\"");

    // Different on platforms: man getsockaddr says "Specifying 0 in
    // hints.ai_socktype indicates that socket addresses of any type can be
    // returned". Linux returns SOCK_STREAM, MacOS returns SOCK_DGRAM and win32
    // returns 0. To be portable we always return 0 in this case, the same as
    // requested by the user.
    if (m_hints.ai_socktype == 0)
        new_res->ai_socktype = 0;
    // Different on platforms: Ubuntu & MacOS return protocol number,
    // win32 returns 0. We just return what was requested by the user.
    new_res->ai_protocol = m_hints.ai_protocol;
    // Different on platforms: Ubuntu returns set flags, MacOS & win32
    // return 0. We just return what was requested by the user.
    new_res->ai_flags = m_hints.ai_flags;
    // Man setaddrinfo says: "If service is NULL, then the port number
    // of the returned socket addresses will be left uninitialized." So
    // we set it definitely to 0.
    if (m_service.empty())
        // port for AF_INET6 is also valid for AF_INET
        reinterpret_cast<sockaddr_in6*>(new_res->ai_addr)->sin6_port = 0;

    TRACE2("syscall ::getaddrinfo() with new_res = ", new_res)
    return new_res;
}


// Compare operator ==
// -------------------
bool CAddrinfo::operator==(const CAddrinfo& a_ai) const {
    if (a_ai.m_res->ai_flags == m_res->ai_flags &&
        a_ai.m_res->ai_family == m_res->ai_family &&
        a_ai.m_res->ai_socktype == m_res->ai_socktype &&
        a_ai.m_res->ai_protocol == m_res->ai_protocol &&
        a_ai.m_res->ai_addrlen == m_res->ai_addrlen &&
        sockaddrcmp(reinterpret_cast<sockaddr_storage*>(a_ai.m_res->ai_addr),
                    reinterpret_cast<sockaddr_storage*>(m_res->ai_addr)))
        return true;

    return false;
}


// Member access operator ->
// -------------------------
::addrinfo* CAddrinfo::operator->() const { return m_res; }

} // namespace upnplib
/// \endcond
