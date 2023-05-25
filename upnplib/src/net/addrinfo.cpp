// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-05-28

#include <upnplib/addrinfo.hpp>
#include <upnplib/sockaddr.hpp>
#include <upnplib/port.hpp>
#include <umock/netdb.hpp>

#include <stdexcept>

namespace upnplib {

// Provide C style addrinfo as class and wrap its system calls
// -----------------------------------------------------------
// Constructor with getting an address information.
CAddrinfo::CAddrinfo(const std::string& a_node, const std::string& a_service,
                     const int a_family, const int a_socktype,
                     const int a_flags, const int a_protocol)
    : m_node(a_node),
      m_service(a_service), m_hints{a_flags, a_family, a_socktype, a_protocol,
                                    {},      nullptr,  nullptr,    nullptr} {
    TRACE2(this, " Construct upnplib::CAddrinfo(..) with arguments")

    // Get new address information from the operating system.
    m_res = this->get_new_addrinfo(); // may throw exception
}

// Copy constructor
CAddrinfo::CAddrinfo(const CAddrinfo& that) {
    TRACE2(this, " Construct copy upnplib::CAddrinfo()")
    m_node = that.m_node;
    m_service = that.m_service;
    m_hints = that.m_hints;
    m_res = this->get_new_addrinfo(); // may throw exception
}

// Copy assignment operator
CAddrinfo& CAddrinfo::operator=(CAddrinfo that) {
    TRACE2(this, " Executing upnplib::CAddrinfo::operator=()")
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

    // by convention, always return *this
    return *this;
}

// Destructor
CAddrinfo::~CAddrinfo() {
    TRACE2(this, " Destruct upnplib::CAddrinfo()")
    TRACE2("Call STL function ::freeaddrinfo() with m_res = ", m_res)
    umock::netdb_h.freeaddrinfo(m_res);
    m_res = nullptr;
}

addrinfo* CAddrinfo::get_new_addrinfo() const {
    TRACE2(this, " Executing upnplib::CAddrinfo::get_new_addrinfo()")
    addrinfo* new_res{nullptr};
    std::string node{m_node};
    int ret{EAI_BADFLAGS};

    // Check if AI_NUMERICHOST flag is set. Due to avoid expensive DNS name
    // resolution we only support this but may be changed if we need name
    // resolution.
    if (!(m_hints.ai_flags & AI_NUMERICHOST))
        goto throw_error1;

    // A numeric ipv6 address must be surounded with brackets.
    if (!m_node.empty() && m_hints.ai_family == AF_INET6) {
        if (m_node.front() != '[' || m_node.back() != ']')
            goto throw_error1;
        else
            // remove surounding brackets for ::getaddrinfo()
            node = m_node.substr(1, m_node.length() - 2);
    }

    // Get new address information with cached hints. This should always return
    // the same address info.
    ret = umock::netdb_h.getaddrinfo(
        node.empty() ? nullptr : node.c_str(),
        m_service.empty() ? nullptr : m_service.c_str(), &m_hints, &new_res);
    if (ret != 0)
        goto throw_error2;

    // Different on platforms: Ubuntu & MacOS return protocol number, win32
    // returns 0. We just return what was requested by the user.
    new_res->ai_protocol = m_hints.ai_protocol;
    // Different on platforms: Ubuntu returns set flags, MacOS & win32 return 0.
    // We just return what was requested by the user.
    new_res->ai_flags = m_hints.ai_flags;
    // Man setaddrinfo says: "If service is NULL, then the port number of the
    // returned socket addresses will be left uninitialized." So we set it
    // definitely to 0.
    if (m_service.empty())
        // port for AF_INET6 is also valid for AF_INET
        ((sockaddr_in6*)new_res->ai_addr)->sin6_port = 0;

    TRACE2("Called STL function ::getaddrinfo() with new_res = ", new_res)
    return new_res;

throw_error1:
    throw std::invalid_argument("[" + std::to_string(__LINE__) +
                                "] ERROR! Failed to get address information: "
                                "invalid numeric node address.");
throw_error2:
    throw std::runtime_error(
        "[" + std::to_string(__LINE__) +
        "] ERROR! Failed to get address information: errid(" +
        std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\"");
}


// Compare operator ==
// ------------------&&
bool CAddrinfo::operator==(const CAddrinfo& a_ai) const {
    if (a_ai.m_res->ai_flags == m_res->ai_flags &&
        a_ai.m_res->ai_family == m_res->ai_family &&
        a_ai.m_res->ai_socktype == m_res->ai_socktype &&
        a_ai.m_res->ai_protocol == m_res->ai_protocol &&
        a_ai.m_res->ai_addrlen == m_res->ai_addrlen &&
        sockaddrcmp((sockaddr_storage*)a_ai.m_res->ai_addr,
                    (sockaddr_storage*)m_res->ai_addr))
        return true;

    return false;
}


// Member access operator ->
// -------------------------
::addrinfo* CAddrinfo::operator->() const { return m_res; }


std::string CAddrinfo::addr_str() const {
    TRACE2(this, " Executing upnplib::CAddrinfo::addr_str()")
    char addrbuf[INET6_ADDRSTRLEN]{};

    if (m_res->ai_family == AF_INET6) {
        sockaddr_in6* sa6 = (sockaddr_in6*)m_res->ai_addr;
        inet_ntop(m_res->ai_family, &sa6->sin6_addr.s6_addr, addrbuf,
                  sizeof(addrbuf));

        return "[" + std::string(addrbuf) + "]";

    } else {

        sockaddr_in* sa = (sockaddr_in*)m_res->ai_addr;
        inet_ntop(m_res->ai_family, &sa->sin_addr.s_addr, addrbuf,
                  sizeof(addrbuf));

        return std::string(addrbuf);
    }
}


uint16_t CAddrinfo::port() const {
    // port for AF_INET6 is also valid for AF_INET
    TRACE2(this, " Executing upnplib::CAddrinfo::port()")
    return ntohs(((sockaddr_in6*)m_res->ai_addr)->sin6_port);
}

} // namespace upnplib
