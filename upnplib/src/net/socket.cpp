// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-05

#include <upnplib/socket.hpp>
#include <upnplib/trace.hpp>
#include <umock/sys_socket.hpp>
#include <umock/stringh.hpp>

#include <stdexcept>

namespace upnplib {

// Initialize and cleanup Microsoft Windows Sockets
// ------------------------------------------------
#ifdef _MSC_VER
CWSAStartup::CWSAStartup() {
    TRACE2(this, " Construct CWSAStartup")
    WSADATA wsaData;
    int rc = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0)
        throw std::runtime_error(
            "UPnPlib ERROR 1003! Failed to initialize Windows "
            "sockets: WSAStartup() returns " +
            std::to_string(rc));
}

CWSAStartup::~CWSAStartup() {
    TRACE2(this, " Destruct CWSAStartup")
    ::WSACleanup();
}
#endif // _MSC_VER


// Free helper functions
// =====================

// Helper inline function to throw an exeption with additional information.
// ------------------------------------------------------------------------
// error number given by WSAGetLastError(), resp. contained in errno is used to
// specify details of the error. It is important that these error numbers
// hasn't been modified by executing other statements.
static inline void throw_error(const std::string& a_errmsg) {
#ifdef _MSC_VER
    throw std::runtime_error(
        a_errmsg + " WSAGetLastError()=" + std::to_string(WSAGetLastError()));
#else
    throw std::runtime_error(a_errmsg + " errno(" + std::to_string(errno) +
                             ")=\"" + umock::string_h.strerror(errno) + "\"");
#endif
}


// upnplib wrapper for the ::getsockname() system function
// -------------------------------------------------------
static int getsockname(SOCKET a_sockfd, struct sockaddr* a_addr,
                       socklen_t* a_addrlen) {
    TRACE("Executing getsockname()");

    int ret = umock::sys_socket_h.getsockname(a_sockfd, a_addr, a_addrlen);
    if (ret == 0)
        return 0;

#ifndef _MSC_VER
    return ret;

#else
    if (WSAGetLastError() != WSAEINVAL) // Error code 10022 not set
        return ret;

    // WSAEINVAL indicates that the socket is unbound. We will return an
    // empty sockaddr with address family set. This is what we get on Unix
    // platforms. On Microsoft Windows we cannot use ::getsockname() like
    // on unix platforms because it returns an error indicating an unbound
    // socket and an untouched socket address storage. Here we have to use
    // an alternative ::getsockopt() that provides additional info with a
    // non-standard option SO_PROTOCOL_INFO.
    ::WSAPROTOCOL_INFO protocol_info{};
    int len{sizeof(protocol_info)}; // May be modified?
    if (umock::sys_socket_h.getsockopt(a_sockfd, SOL_SOCKET, SO_PROTOCOL_INFO,
                                       &protocol_info, &len) != 0)
        return ENOBUFS; // Insufficient resources were available in the system
                        // to perform the operation.

    memset(a_addr, 0, *a_addrlen);
    // Microsoft itself defines sockaddr.sa_family as ushort, so the typecast
    // from int should not do any harm.
    a_addr->sa_family =
        static_cast<unsigned short>(protocol_info.iAddressFamily);
    return 0;
#endif
}


// CSocket_basic class
// ===================
// Default constructor for an empty socket object
CSocket_basic::CSocket_basic(){
    TRACE2(this, " Construct default CSocket_basic()") //
}

// Constructor with given file desciptor
CSocket_basic::CSocket_basic(SOCKET a_sfd) {
    TRACE2(this, " Construct CSocket_basic(SOCKET)")

    // Check if we have a valid socket file descriptor
    int so_option{-1};
    socklen_t optlen{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(a_sfd, SOL_SOCKET, SO_ERROR,
                                       (char*)&so_option, &optlen) != 0)
        throw_error("UPnPlib ERROR 1014! Failed to create socket:");

    m_sfd = a_sfd;
}

// Destructor
CSocket_basic::~CSocket_basic(){
    TRACE2(this, " Destruct CSocket_basic()") //
}

// Get the raw socket file descriptor
CSocket_basic::operator SOCKET&() const {
    TRACE2(this, " Executing CSocket_basic::operator SOCKET&() (get "
                 "raw socket fd)")
    // There is no problem with cast here. We cast to const so we can only read.
    return const_cast<SOCKET&>(m_sfd);
}

// Getter
// ------
sa_family_t CSocket_basic::get_family() const {
    TRACE2(this, " Executing CSocket_basic::get_family()")
    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("UPnPlib ERROR 1032! Failed to get socket address family:");

    return ss.ss_family;
}

std::string CSocket_basic::get_addr_str() const {
    TRACE2(this, " Executing CSocket::get_addr_str()")

    // Get address from socket file descriptor
    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("UPnPlib ERROR 1001! Failed to get socket address/port:");

    char addr_buf[INET6_ADDRSTRLEN]{};
    const char* ret{nullptr};
    switch (ss.ss_family) {
    case AF_INET6:
        ret = ::inet_ntop(AF_INET6, &((sockaddr_in6*)&ss)->sin6_addr, addr_buf,
                          sizeof(addr_buf));
        break;
    case AF_INET:
        ret = ::inet_ntop(AF_INET, &((sockaddr_in*)&ss)->sin_addr, addr_buf,
                          sizeof(addr_buf));
        break;
    default:
        throw std::runtime_error(
            "UPnPlib ERROR 1024! Failed to get a socket address string (IP "
            "address): invalid address family " +
            std::to_string(ss.ss_family));
    }

    if (ret == nullptr)
        throw_error("UPnPlib ERROR 1025! Failed to get a socket address string "
                    "(IP address):");

    // Surround IPv6 address with '[' and ']'
    return (ss.ss_family == AF_INET6) ? '[' + std::string(addr_buf) + ']'
                                      : std::string(addr_buf);
}

uint16_t CSocket_basic::get_port() const {
    TRACE2(this, " Executing CSocket_basic::get_port()")

    // Get port from socket file descriptor
    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("UPnPlib ERROR 1026! Failed to get socket port:");

    // Because sin6_port and sin_port are as union at the same memory location
    // this can be used for AF_INET6 and AF_INET port queries.
    return ntohs(((sockaddr_in6*)&ss)->sin6_port);
}

int CSocket_basic::get_type() const {
    TRACE2(this, " Executing CSocket_basic::get_type()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_TYPE,
                                       (char*)&so_option, &len) != 0)
        throw_error("UPnPlib ERROR 1030! Failed to get socket option SO_TYPE:");

    return so_option;
}

int CSocket_basic::get_sockerr() const {
    TRACE2(this, " Executing CSocket_basic::get_sockerr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_ERROR,
                                       (char*)&so_option, &len) != 0)
        throw_error(
            "UPnPlib ERROR 1011! Failed to get socket option SO_ERROR:");

    return so_option;
}

bool CSocket_basic::is_reuse_addr() const {
    TRACE2(this, " Executing CSocket_basic::is_reuse_addr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_REUSEADDR,
                                       (char*)&so_option, &len) != 0)
        throw_error(
            "UPnPlib ERROR 1013! Failed to get socket option SO_REUSEADDR:");

    return so_option;
}


// CSocket class
// =============
// Default constructor for an empty socket object
CSocket::CSocket(){
    TRACE2(this, " Construct default CSocket()") //
}

// Constructor for new socket file descriptor
CSocket::CSocket(sa_family_t a_family, int a_type) {
    TRACE2(this, " Construct CSocket(af, type)")

    if (a_family != AF_INET6 && a_family != AF_INET)
        throw std::invalid_argument("UPnPlib ERROR 1015! Failed to create "
                                    "socket: invalid address family " +
                                    std::to_string(a_family));
    if (a_type != SOCK_STREAM && a_type != SOCK_DGRAM)
        throw std::invalid_argument("UPnPlib ERROR 1016! Failed to create "
                                    "socket: invalid socket type " +
                                    std::to_string(a_type));

    // Get new socket file descriptor.
    SOCKET sfd = ::socket(a_family, a_type, 0);
    if (sfd == INVALID_SOCKET)
        throw_error("UPnPlib ERROR 1017! Failed to create socket:");

    int so_option{0};
    constexpr socklen_t optlen{sizeof(so_option)};

    // Reset SO_REUSEADDR on all platforms if it should be set by default. This
    // is unclear on WIN32. See note below.
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&so_option,
                     optlen) != 0) {
        CLOSE_SOCKET_P(sfd);
        throw_error(
            "UPnPlib ERROR 1018! Failed to set socket option SO_REUSEADDR:");
    }

#ifdef _MSC_VER
    // Set socket option SO_EXCLUSIVEADDRUSE on Microsoft Windows.
    // THIS IS AN IMPORTANT SECURITY ISSUE! Lock at
    // REF: [Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE]
    // (https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse#application-strategies)
    so_option = 1; // Set SO_EXCLUSIVEADDRUSE
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::setsockopt(sfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&so_option,
                     optlen) != 0) {
        CLOSE_SOCKET_P(sfd);
        throw_error("UPnPlib ERROR 1019! Failed to set socket option "
                    "SO_EXCLUSIVEADDRUSE:");
    }
#endif

    // Try to set socket option IPV6_V6ONLY to false if possible, means
    // allowing IPv4 and IPv6. Although it makes no sense to enable
    // IPV6_V6ONLY on an interface that can only use IPv4 we must reset it on
    // Microsoft Windows because it is set there.
#ifndef _MSC_VER
    if (a_family == AF_INET6) {
#endif
        so_option = 0;
        // Type cast (char*)&so_option is needed for Microsoft Windows.
        if (::setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&so_option,
                         optlen) != 0) {
            CLOSE_SOCKET_P(sfd);
            throw_error(
                "UPnPlib ERROR 1020! Failed to set socket option IPV6_V6ONLY:");
        }
#ifndef _MSC_VER
    }
#endif

    // Store socket file descriptor
    m_sfd = sfd;
}

// Move constructor
CSocket::CSocket(CSocket&& that) {
    TRACE2(this, " Construct move CSocket()")
    m_sfd = that.m_sfd;
    that.m_sfd = INVALID_SOCKET;

    // Following variables are protected
    std::scoped_lock lock(m_listen_mutex);
    m_listen = that.m_listen;
    that.m_listen = false;
}

// Assignment operator (parameter as value)
CSocket& CSocket::operator=(CSocket that) {
    TRACE2(this, " Executing CSocket::operator=()")
    std::swap(m_sfd, that.m_sfd);

    // Following variables are protected
    std::scoped_lock lock(m_listen_mutex);
    std::swap(m_listen, that.m_listen);

    return *this;
}

// Destructor
CSocket::~CSocket() {
    TRACE2(this, " Destruct CSocket()")
    ::shutdown(m_sfd, SHUT_RDWR);
    CLOSE_SOCKET_P(m_sfd);
}

// Setter
// ------
#if 0
void CSocket::set_reuse_addr(bool a_reuse) {
    // Set socket option SO_REUSEADDR on other platforms.
    // --------------------------------------------------
    // REF: [How do SO_REUSEADDR and SO_REUSEPORT differ?]
    // (https://stackoverflow.com/a/14388707/5014688)
    so_option = a_reuse_addr ? 1 : 0;
    // Type cast (char*)&so_reuseaddr is needed for Microsoft Windows.
    if (::setsockopt(m_listen_sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&so_option,
                     optlen) != 0)
        throw_error(
            "[Server] ERROR! Failed to set socket option SO_REUSEADDR:");
}
#endif

// Bind socket to local address
// REF: [Bind: Address Already in Use]
// (https://hea-www.harvard.edu/~fine/Tech/addrinuse.html)
void CSocket::bind(const std::string& a_node, const std::string& a_port,
                   const int a_flags) {
    TRACE2(this, " Executing CSocket::bind()")

    // Protect binding.
    std::scoped_lock lock(m_bound_mutex);

    const sa_family_t addr_family = this->get_family();

    // With address family AF_INET6 and not passive mode we always set
    // IPV6_V6ONLY to true. This is the behavior on Unix platforms when binding
    // the address and cannot be modified afterwards. MacOS does not modify the
    // flag with binding. To be portable with same behavior on all platforms we
    // set the flag before binding.
    if (addr_family == AF_INET6 && (a_flags & AI_PASSIVE) == 0) {
        // Don't use 'this->set_v6only(true)' because binding is protected with
        // a mutex and we will get a deadlock because of using
        // 'this->is_bound()' in 'this->set_v6only(true)'.
        constexpr int so_option{1}; // true
        // Type cast (char*)&so_option is needed for Microsoft Windows.
        if (::setsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&so_option,
                         sizeof(so_option)) != 0)
            throw_error(
                "UPnPlib ERROR 1007! Failed to set socket option IPV6_V6ONLY:");
    }

    // Here we bind the socket to an address
    const CAddrinfo ai(a_node, a_port, addr_family, this->get_type(),
                       AI_NUMERICHOST | AI_NUMERICSERV | a_flags);
    // Type cast socklen_t is needed for Microsoft Windows.
    if (::bind(m_sfd, ai->ai_addr, (socklen_t)ai->ai_addrlen) != 0)
        throw_error("UPnPlib ERROR 1008! Failed to bind socket to an address:");
}

// Set socket to listen
void CSocket::listen() {
    TRACE2(this, " Executing CSocket::listen()")

    // Protect set listen and storing its state (m_listen).
    std::scoped_lock lock(m_listen_mutex);

    if (m_listen)
        return;

    // Second argument backlog (maximum length of the queue for pending
    // connections) is hard coded set to 1 for now.
    if (::listen(m_sfd, 1) != 0)
        throw_error("UPnPlib ERROR 1034! Failed to set socket to listen:");

    m_listen = true;
}

// Set IPV6_V6ONLY
void CSocket::set_v6only(const bool a_opt) {
    TRACE2(this, " Executing CSocket::set_ipv6_v6only()")

    // Needed to have a valid argument for setsockopt()
    const int so_option{a_opt};

    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (this->get_family() == AF_INET6 && !this->is_bound() &&
        ::setsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&so_option,
                     sizeof(so_option)) != 0)
        throw_error(
            "UPnPlib ERROR 1006! Failed to set socket option IPV6_V6ONLY:");
}

// Getter
// ------
sa_family_t CSocket::get_family() const {
    TRACE2(this, " Executing CSocket::get_family()")
    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("UPnPlib ERROR 1027! Failed to get socket address family:");

    return ss.ss_family;
}

uint16_t CSocket::get_port() const {
    TRACE2(this, " Executing CSocket::get_port()")

    // Get port from socket file descriptor
    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("UPnPlib ERROR 1031! Failed to get socket port:");

    // Because sin6_port and sin_port are as union at the same memory location
    // this can be used for AF_INET6 and AF_INET port queries.
    return ntohs(((sockaddr_in6*)&ss)->sin6_port);
}

bool CSocket::is_v6only() const {
    TRACE2(this, " Executing CSocket::is_v6only()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("UPnPlib ERROR 1028! Failed to get socket "
                                 "option IPV6_V6ONLY: \"Bad file descriptor\"");

    int so_option{0};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    umock::sys_socket_h.getsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                                   (char*)&so_option, &len);

    return so_option;
}

bool CSocket::is_bound() const {
    // We get the socket address from the file descriptor and check if its
    // address and port are all zero. We have to do this different for AF_INET6
    // and AF_INET.
    TRACE2(this, " Executing CSocket::is_bound()")

    // binding is protected.
    std::scoped_lock lock(m_bound_mutex);

    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified
    if (upnplib::getsockname(m_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error(
            "UPnPlib ERROR 1010! Failed to get socket status 'is_bound':");

    switch (ss.ss_family) {
    case AF_INET6: {
        // The IPv6 address has 16 bytes so we simply compare it with a null
        // bytes address.
        unsigned char sin6_addr0[16]{};
        sockaddr_in6* sa_in6 = (sockaddr_in6*)&ss;
        // If address and port are 0 then the socket isn't bound to an address.
        return (memcmp(sa_in6->sin6_addr.s6_addr, sin6_addr0,
                       sizeof(sin6_addr0)) == 0 &&
                sa_in6->sin6_port == 0)
                   ? false
                   : true;
    }
    case AF_INET: {
        sockaddr_in* sa_in = (sockaddr_in*)&ss;
        // If address and port are 0 then the socket isn't bound to an address.
        return (sa_in->sin_addr.s_addr == 0 && sa_in->sin_port == 0) ? false
                                                                     : true;
    }
    default:
        throw std::runtime_error("UPnPlib ERROR 1029! Failed to check if "
                                 "socket is bound to an addess: "
                                 "invalid address family " +
                                 std::to_string(ss.ss_family));
    }
}

bool CSocket::is_listen() const {
    TRACE2(this, " Executing CSocket::is_listen()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(
            "UPnPlib ERROR 1035! Failed to get socket option "
            "'is_Listen': \"Bad file descriptor\"");

    // m_listen is protected.
    std::scoped_lock lock(m_listen_mutex);
    return m_listen;
}


} // namespace upnplib
