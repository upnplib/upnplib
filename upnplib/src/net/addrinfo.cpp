// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-15

#include <upnplib/addrinfo.hpp>
#include <upnplib/port.hpp>

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

CAddrinfo::~CAddrinfo() {
    TRACE2(this, " Destruct upnplib::CAddrinfo()")
    TRACE2("Call freeaddrinfo() with m_res = ", m_res)
    ::freeaddrinfo(m_res);
    m_res = nullptr;
}

addrinfo* CAddrinfo::get_new_addrinfo() {
    TRACE2(this, " Executing upnplib::CAddrinfo::get_new_addrinfo()")

    // Get new address information with cached hints. This should always return
    // the same address info.
    addrinfo* new_res{nullptr};

    int ret = ::getaddrinfo(m_node.empty() ? nullptr : m_node.c_str(),
                            m_service.empty() ? nullptr : m_service.c_str(),
                            &m_hints, &new_res);
    if (ret != 0) {
        throw std::runtime_error(
            "[" + std::to_string(__LINE__) +
            "] ERROR! Failed to get address information: errid(" +
            std::to_string(ret) + ")=\"" + ::gai_strerror(ret) + "\"");
    }

    // Different on platforms: Ubuntu & MacOS return protocol number, win32
    // returns 0. We just return what was requested by the user.
    new_res->ai_protocol = m_hints.ai_protocol;
    // Different on platforms: Ubuntu returns set flags, MacOS & win32 return 0.
    // We just return what was requested by the user.
    new_res->ai_flags = m_hints.ai_flags;

    TRACE2("Called getaddrinfo() with new_res = ", new_res)
    return new_res;
}

addrinfo* CAddrinfo::operator->() const { return m_res; }

std::string CAddrinfo::addr_str() const {
    TRACE2(this, " Executing upnplib::CAddrinfo::addr_str()")
    char addrbuf[INET6_ADDRSTRLEN]{};

    if (m_res->ai_family == AF_INET6) {
        sockaddr_in6* sa6 = (sockaddr_in6*)m_res->ai_addr;
        inet_ntop(m_res->ai_family, &sa6->sin6_addr.s6_addr, addrbuf,
                  sizeof(addrbuf));
    } else {
        sockaddr_in* sa = (sockaddr_in*)m_res->ai_addr;
        inet_ntop(m_res->ai_family, &sa->sin_addr.s_addr, addrbuf,
                  sizeof(addrbuf));
    }

    return std::string(addrbuf);
}

uint16_t CAddrinfo::port() const {
    // port for AF_INET6 is also valid for AF_INET
    TRACE2(this, " Executing upnplib::CAddrinfo::port()")
    return ntohs(((sockaddr_in6*)m_res->ai_addr)->sin6_port);
}

} // namespace upnplib
