// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-28
/*!
 * \file
 * \brief Definition of the Addrinfo class and free helper functions.
 */

#include <upnplib/addrinfo.hpp>

#include <umock/netdb.hpp>
#include <cstring>

namespace upnplib {

// CAddrinfo class to wrap ::addrinfo() system calls
// =================================================
// On Microsoft Windows this needs to have Windows Sockets initialized with
// WSAStartup().

// Constructor for getting an address information with service name that can
// also be a port number string.
CAddrinfo::CAddrinfo(std::string_view a_node, std::string_view a_service,
                     const int a_family, const int a_socktype,
                     const int a_flags, const int a_protocol)
    : m_node(a_node), m_service(a_service) {
    TRACE2(this, " Construct CAddrinfo() with service")
    this->set_ai_flags(a_family, a_socktype, a_flags, a_protocol);
}

// Constructor for getting an address information from only a netaddress.
// ----------------------------------------------------------------------
CAddrinfo::CAddrinfo(std::string_view a_node, const int a_family,
                     const int a_socktype, const int a_flags,
                     const int a_protocol)
    : m_node(a_node), m_service("0") {
    // Just do a simple check for a possible port and split it from address
    // string. Detailed tests will be done with this->init().
    TRACE2(this, " Construct CAddrinfo() without service")

    size_t pos;
    // The smalest valid netaddress is "[::]".
    if (m_node.size() < 4 || (m_node.front() == '[' && m_node.back() == ']')) {
        // IPv6 without port, use given address string
        goto set_flags;
    }
    pos = m_node.rfind("]:"); // noexcept
    if (pos != m_node.npos) {
        // IPv6 with port, split the address string. substr() throws exception
        // std::out_of_range if pos > size(). We have at least 4 character. pos
        // is 0-based, size() is 1-based so with "]:" is size()==2, pos==0,
        // pos+2==size() does not throw.
        m_service = m_node.substr(pos + 2);
        m_node = m_node.substr(0, pos + 1);
        goto set_flags;
    }
    pos = m_node.find_last_of(']'); // noexcept
    if (pos != m_node.npos) {
        // Maybe IPv6 with any remaining character, split address string
        m_service = m_node.substr(pos + 1);
        m_node = m_node.substr(0, pos + 1);
        goto set_flags;
    }
    pos = m_node.find_last_of(':'); // noexcept
    if (pos != m_node.npos) {
        // IPv4 or URL name with port, split address string
        m_service = m_node.substr(pos + 1);
        m_node = m_node.substr(0, pos);
    }

set_flags:
    this->set_ai_flags(a_family, a_socktype, a_flags, a_protocol);
}

// Helper method for common tasks on different constructors
// --------------------------------------------------------
void CAddrinfo::set_ai_flags(const int a_family, const int a_socktype,
                             const int a_flags, const int a_protocol) noexcept {
    // I cannot use the initialization list of the constructor because the
    // member order in the structure addrinfo is different on Linux, MacOS and
    // win32. I have to use the member names to initialize them, what's not
    // possible for structures in the constructors init list.
    m_hints.ai_flags = a_flags;
    m_hints.ai_family = a_family;
    m_hints.ai_socktype = a_socktype;
    m_hints.ai_protocol = a_protocol;

    // Correct weak hints:
    // Always call is_netaddr() with AF_UNSPEC (default argument) to check
    // all types. We always set AI_NUMERICHOST if it match, no matter what
    // 'a_family' was requested by the caller. But if AF_UNSPEC was requested,
    // the found address family will also be set.
    const sa_family_t addr_family = is_netaddr(m_node); // noexcept
    if (addr_family != AF_UNSPEC) { // Here we have a numeric netaddress
        m_hints.ai_flags |= AI_NUMERICHOST;
        if (m_hints.ai_family == AF_UNSPEC) { // Correct ai_family, we know it
            m_hints.ai_family = addr_family;
        }
    }
}


// Destructor
// ----------
/// \cond
CAddrinfo::~CAddrinfo() {
    TRACE2(this, " Destruct CAddrinfo()")
    this->free_addrinfo();
}
/// \endcond

// Private method to free allocated memory for address information
// ---------------------------------------------------------------
void CAddrinfo::free_addrinfo() noexcept {
    if (m_res != &m_hints) {
        TRACE2("syscall ::freeaddrinfo() with m_res = ", m_res)
        umock::netdb_h.freeaddrinfo(m_res);
        m_res = &m_hints;
    }
}


// Member access operator ->
// -------------------------
::addrinfo* CAddrinfo::operator->() const noexcept { return m_res; }


// Setter to initialize an addrinfo and set it to the object
// ---------------------------------------------------------
// Get address information with cached hints.
void CAddrinfo::init() {
    TRACE2(this, " Executing CAddrinfo::init()")

    // Temporary working copies: modified node and hints to use for syscall
    // ::getaddrinfo().
    std::string node;
    ::addrinfo hints{m_hints};

    if (is_netaddr(m_node, AF_INET6) == AF_INET6) {
        // Here we have only ipv6 node strings representing a numeric ip
        // address (netaddress) without port that is at least "[::]". Remove
        // surounding brackets for ::getaddrinfo()
        node = m_node.substr(1, m_node.length() - 2);
    } else if (is_netaddr('[' + m_node + ']', AF_INET6) == AF_INET6) {
        // Netaddresses without brackets would be accepted by ::getaddrinfo().
        // So I make them invalid.
        hints.ai_flags |= AI_NUMERICHOST;
        node = m_node + "(no_brackets)";
    } else if (m_node.find_first_of("[]:") != std::string::npos) { // noexcept
        // An address string without port (m_node is without port) containing
        // one of theese characters cannot be a valid alphanumeric URL. It can
        // only be a numeric address (or be invalid).
        hints.ai_flags |= AI_NUMERICHOST;
        node = m_node;
    } else {
        // Here we have an ipv4 netaddress or a non numeric node name (no
        // netaddress). Is it a valid (maybe alphanumeric) node name?
        // ::getaddrinfo() shall decide it.
        node = m_node;
    }
    const char* c_node = node.empty() ? nullptr : node.c_str();
    std::string node_out = node.empty() ? "nullptr" : "\"" + node + "\"";

    std::string service;
    switch (is_numport(m_service)) {
    case 0:
        // Valid numeric port number
        service = m_service;
        hints.ai_flags |= AI_NUMERICSERV;
        break;
    case 1:
        // Invalid numeric port number > 65535
        service = m_service + "(invalid)";
        hints.ai_flags |= AI_NUMERICSERV;
        break;
    default:
        // Any alphanumeric port name
        service = m_service;
    }

    ::addrinfo* new_res{nullptr};

    // syscall ::getaddrinfo() with prepared arguments
    // -----------------------------------------------
    int ret =
        umock::netdb_h.getaddrinfo(c_node, service.c_str(), &hints, &new_res);

    // Very helpful for debugging to see what is given to ::getaddrinfo()
    // clang-format off
    UPNPLIB_LOGINFO << "MSG1114: syscall ::getaddrinfo(" << node_out
        << ", " << "\"" << m_service << "\", "
        << &hints << ", " << &new_res
        << ") node=\"" << m_node << "\", "
        << (hints.ai_flags & AI_NUMERICHOST ? "AI_NUMERICHOST, " : "")
        << (hints.ai_flags & AI_NUMERICSERV ? "AI_NUMERICSERV, " : "")
        << (hints.ai_flags & AI_PASSIVE ? "AI_PASSIVE, " : "")
        << (m_hints.ai_family == AF_INET6 ? "AF_INET6" :
                (m_hints.ai_family == AF_INET ? "AF_INET" :
                    (m_hints.ai_family == AF_UNSPEC ? "AF_UNSPEC" :
                        "hints.ai_family=" + std::to_string(m_hints.ai_family))))
        << (ret != 0
            ? ". Get ERROR"
            : ". Get \"" + to_netaddrp(reinterpret_cast
              <const sockaddr_storage*>(new_res->ai_addr)) + "\"") << "\n";

    if (ret == EAI_SERVICE || ret == EAI_NONAME || ret == EAI_AGAIN || ret == EAI_NODATA) {
        // Servname not supported for ai_socktype (EAI_SERVICE). Node or
        // service not known (EAI_NONAME). The name server returned a temporary
        // failure indication, try again later (EAI_AGAIN). No address
        // associated with hostname (EAI_NODATA). Error numbers definded in
        // netdb.h. Maybe an alphanumeric node name that cannot be resolved
        // (e.g. by DNS)? Anyway, the user has to decide what to do. Because
        // this depends on extern available DNS server the error can occur
        // unexpectedly at any time. We have no influence on it but I will give
        // an extended error message.
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
            std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\"\n");
    }
    // Different on platforms: man getsockaddr says "Specifying 0 in
    // hints.ai_socktype indicates that socket addresses of any type can be
    // returned". Linux returns SOCK_STREAM, MacOS returns SOCK_DGRAM and
    // win32 returns 0. To be portable we always return 0 in this case, the
    // same as requested by the user.
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
    // If init() is called the second time then m_res still points to the
    // previous allocated memory. To avoid a memory leak it must be freed
    // before pointing to the new allocated memory.
    this->free_addrinfo();
    // finaly point to the new address information from the operating
    // system.
    m_res = new_res;
}

// Getter for the assosiated ip address with port
// ----------------------------------------------
// e.g. "[2001:db8::2]:50001" or "192.168.254.253:50001".
Netaddr CAddrinfo::netaddr() const noexcept {
    // TRACE not usable with chained output.
    // TRACE2(this, " Executing SSockaddr::get_netaddrp()")
    Netaddr netaddr;
    if (m_res == &m_hints)
        return netaddr; // no information available

    netaddr.m_netaddrp = to_netaddrp( // noexcept
        reinterpret_cast<sockaddr_storage*>(m_res->ai_addr));

    return netaddr;
}

} // namespace upnplib
