// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-09
/*!
 * \file
 * \brief Definition of the Addrinfo class and free helper functions.
 */

#include <upnplib/addrinfo.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>
#include <umock/netdb.hpp>

#include <cstring>

namespace {

/*!
 * \brief Check for a numeric netaddress and return its address family
 * <!--   ------------------------------------------------------------ -->
 * \ingroup upnplib-addrmodul
 *
 * Checks if a string represents a numeric IPv6 or IPv4 address without port and
 * determines its address family.
 *
 * \returns
 *  On success: Address family AF_INET6 or AF_INET the address belongs to\n
 *  On error: AF_UNSPEC, the address is alphanumeric (maybe a DNS name?)
 *
 * \exception ... None, function is declared 'noexcept'
 */
// I simply use the system function %inet_pton() to check if the node string is
// accepted.
sa_family_t is_netaddr(const std::string& a_node,
                       const int a_addr_family = AF_UNSPEC) noexcept {
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

} // namespace

namespace upnplib {

// CAddrinfo class to wrap ::addrinfo() system calls
// =================================================
// On Microsoft Windows this needs to have Windows Sockets initialized with
// WSAStartup().

// Constructor for getting an address information with port number string.
CAddrinfo::CAddrinfo(const std::string& a_node, const std::string& a_service,
                     const int a_family, const int a_socktype,
                     const int a_flags, const int a_protocol)
    : m_node(a_node), m_service(a_service) {
    TRACE2(this, " Construct CAddrinfo(..) with arguments")

    // I cannot use the initialization list of the constructor because the
    // member order in the structure addrinfo is different on Linux, MacOS and
    // win32. I have to use the member names to initialize them, what's not
    // possible for structures in the constructors init list.
    m_hints.ai_flags = a_flags;
    m_hints.ai_family = a_family;
    m_hints.ai_socktype = a_socktype;
    m_hints.ai_protocol = a_protocol;
    m_hints.ai_addrlen = sizeof(this->ss);
    m_hints.ai_addr = &this->sa;
    m_hints.ai_canonname = m_empty_c_str;
    m_hints.ai_next = nullptr;

    // Correct weak hints:
    // Always call is_netaddr() with AF_UNSPEC (default argument) to check
    // all types. We always set AI_NUMERICHOST if it match, no matter what
    // 'a_family' was reqested by the caller. But if AF_UNSPEC was requested,
    // the found address family will also be set.
    const sa_family_t addr_family = is_netaddr(m_node);
    if (addr_family != AF_UNSPEC) {
        m_hints.ai_flags |= AI_NUMERICHOST;
        if (m_hints.ai_family == AF_UNSPEC) {
            m_hints.ai_family = addr_family;
        }
    }
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
    // I cannot just copy the structure that m_res pointed to (*m_res =
    // *that.m_res;). Copy works but MS Windows failed to destruct it with
    // freeaddrinfo(m_res);. It throws an exception "A non-recoverable error
    // occurred during a database lookup.". Seems there are also pointer within
    // the addrinfo structure that are not deeply copied. So we have to go the
    // hard way with getaddrinfo() and free it with freeaddrinfo(),

    TRACE2(this, " Construct copy CAddrinfo()")
    m_node = that.m_node;
    m_service = that.m_service;
    m_hints = that.m_hints;
    // Next call is capable to throw exceptions but will not do in this case.
    // The call was already successful done with initializing the object.
    // Calling it again with unmodified hints should also always succeed.
    if (that.m_res != &that.m_hints)
        this->get_addrinfo();
}


// Copy assignment operator
// ------------------------
CAddrinfo& CAddrinfo::operator=(CAddrinfo that) noexcept {
    TRACE2(this, " Executing CAddrinfo::operator=()")
    // The argument by value ('that') was copied to the stack by the copy
    // constructor. It contains also a pointer (m_res) to a new allocated
    // addrinfo.
    if (m_res != &m_hints && that.m_res != &that.m_hints)
        std::swap(m_res, that.m_res);
    else
        this->free_addrinfo(); // noexcept
    // The no longer needed current m_res pointer has been swapped to the
    // stack and its resource will be deallocated by the destructor of the
    // 'that' object when leaving this function.

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
    this->free_addrinfo();
}

// Private method to free allocated memory for address information
// ---------------------------------------------------------------
void CAddrinfo::free_addrinfo() noexcept {
    if (m_res != &m_hints) {
        TRACE2("syscall ::freeaddrinfo() with m_res = ", m_res)
        umock::netdb_h.freeaddrinfo(m_res);
        m_res = &m_hints;
        memset(&this->ss, 0, sizeof(this->ss));
    }
}


// Compare operator ==
// -------------------
bool CAddrinfo::operator==(const CAddrinfo& a_ai) const noexcept {
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
::addrinfo* CAddrinfo::operator->() const noexcept { return m_res; }


// Getter to get an addrinfo and set it to the object
// --------------------------------------------------
// Get address information with cached hints.
void CAddrinfo::get_addrinfo() {
    TRACE2(this, " Executing CAddrinfo::get_addrinfo()")

    // Temporary working copies: modified node and hints to use for syscall
    // ::getaddrinfo().
    std::string node;
    addrinfo hints = m_hints;
    hints.ai_addrlen = 0;
    hints.ai_addr = nullptr;
    hints.ai_canonname = nullptr;
    hints.ai_next = nullptr;

    if (is_netaddr(m_node, AF_INET6) == AF_INET6) {
        // Here we have only ipv6 node strings representing a numeric ip
        // address (netaddress) that is at least "[::]". Remove surounding
        // brackets for ::getaddrinfo()
        node = m_node.substr(1, m_node.length() - 2);
    } else if (m_node == "::1") {
        // Special case that isn't a valid netaddress but would be accepted by
        // ::getaddrinfo(). So I make it also invalid for it.
        node = "::1(no_brackets)";
    } else {
        // Here we have a non numeric node name (no netaddress). Is it a valid
        // alphanumeric node name? ::getaddrinfo() shall decide it.
        node = m_node;
    }
    const char* c_node = node.empty() ? nullptr : node.c_str();
    std::string node_out = node.empty() ? "nullptr" : "\"" + node + "\"";

    addrinfo* new_res{nullptr};

    // syscall ::getaddrinfo() with prepared arguments
    // -----------------------------------------------
    int ret =
        umock::netdb_h.getaddrinfo(c_node, m_service.c_str(), &hints, &new_res);

    // Very helpful for debugging to see what is given to ::getaddrinfo()
    // clang-format off
    UPNPLIB_LOGINFO << "MSG1114: syscall ::getaddrinfo("
        << node_out << ", "
        << "\"" + m_service + "\", "
        << &hints << ", " << &new_res << ") Using node=\""
        << m_node << "\", hints.ai_flags="
        << m_hints.ai_flags << ", hints.ai_family="
        << m_hints.ai_family
        << (ret != 0
            ? ". Get ERROR"
            : ". Get \"" + to_addrp_str(reinterpret_cast
              <const sockaddr_storage*>(new_res->ai_addr)) + "\"") << "\n";

    if (ret == EAI_NONAME || ret == EAI_AGAIN) {
        // Node or service not known (EAI_NONAME) or the name server returned a
        // temporary failure indication. Try again later (EAI_AGAIN). Maybe an
        // alphanumeric node name that cannot be resolved (e.g. by DNS)?
        // Anyway, the user has to decide what to do. Because this depends on
        // extern available DNS server the error can occur unexpectedly at any
        // time. We have no influence on it.
        throw std::runtime_error(UPNPLIB_LOGEXCEPT + "MSG1111: errid(" +
             std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\", " +
             ((hints.ai_family == AF_UNSPEC) ? "IPv?_" :
             ((hints.ai_family == AF_INET6) ? "IPv6_" : "IPv4_")) +
             ((hints.ai_flags & AI_NUMERICHOST) ? "numeric_host=\"" : "alphanum_name=\"") +
              node + "\", service=\"" +
              m_service + "\"" +
             ((hints.ai_flags & AI_PASSIVE) ? ", passive_listen" : "") +
             ((hints.ai_flags & AI_NUMERICHOST) ? "" : ", (maybe DNS query temporary failed?)"));
    }
    // clang-format on

    if (ret != 0) {
        throw std::invalid_argument(
            UPNPLIB_LOGEXCEPT +
            "MSG1037: Failed to get address information: errid(" +
            std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\"");
    }
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
    // Man getaddrinfo says: "If service is NULL, then the port number
    // of the returned socket addresses will be left uninitialized." So
    // we set it definitely to 0.
    if (m_service.empty())
        // port for AF_INET6 is also valid for AF_INET
        reinterpret_cast<sockaddr_in6*>(new_res->ai_addr)->sin6_port = 0;

    TRACE2("syscall ::getaddrinfo() with new_res = ", new_res)
    // finaly store the address information from the operating system.
    this->free_addrinfo();
    m_res = new_res;
    // Copy socket address from the address info to this socket address:
    std::memcpy(&this->ss, m_res->ai_addr, m_res->ai_addrlen);
}


} // namespace upnplib
