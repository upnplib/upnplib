// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-24

#include <upnplib/socket.hpp>
#include <upnplib/general.hpp>
#include <upnplib/sockaddr.hpp>
#include <umock/sys_socket.hpp>
#include <umock/stringh.hpp>
#ifdef _MSC_VER
#include <umock/winsock2.hpp>
#endif

#include <stdexcept>
#include <iostream>

namespace upnplib {

// Initialize and cleanup Microsoft Windows Sockets
// ------------------------------------------------
#ifdef _MSC_VER
CWSAStartup::CWSAStartup() {
    TRACE2(this, " Construct CWSAStartup")
    WSADATA wsaData;
    int rc = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0)
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1003: Failed to initialize Windows "
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

// Free function to get socket type string (eg. "SOCK_STREAM") from value
// ----------------------------------------------------------------------
#if 0 // When used then tests are needed.
std::string to_socktype_str(const int socktype) {
    switch (socktype) {
    case AF_INET6:
        return "AF_INET6";
    case AF_INET:
        return "AF_INET";
    case AF_UNSPEC:
        return "AF_UNSPEC";
    default:
        return "AF_" + std::to_string(socktype);
    }
}
#endif


// Helper inline function to throw an exeption with additional information.
// ------------------------------------------------------------------------
// error number given by WSAGetLastError(), resp. contained in errno is used to
// specify details of the error. It is important that these error numbers
// hasn't been modified by executing other statements.
static inline void throw_error(const std::string& a_errmsg) {
#ifdef _MSC_VER
    throw std::runtime_error(
        UPNPLIB_LOGEXCEPT + a_errmsg + " WSAGetLastError()=" +
        std::to_string(umock::winsock2_h.WSAGetLastError()));
#else
    throw std::runtime_error(UPNPLIB_LOGEXCEPT + a_errmsg + " errno(" +
                             std::to_string(errno) + ")=\"" +
                             umock::string_h.strerror(errno) + "\"\n");
#endif
}


// upnplib wrapper for the ::getsockname() system function
// -------------------------------------------------------
static int getsockname(SOCKET a_sockfd, sockaddr* a_addr,
                       socklen_t* a_addrlen) {
    TRACE("Executing getsockname()");

    int ret = umock::sys_socket_h.getsockname(a_sockfd, a_addr, a_addrlen);
    if (ret == 0) {
        return 0;
    }
#ifndef _MSC_VER
    return ret;

#else
    if (umock::winsock2_h.WSAGetLastError() != WSAEINVAL) // Error 10022 not set
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
        return WSAENOBUFS; // Insufficient resources were available in the
                           // system to perform the operation.

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
    TRACE2(this, " Calling system funtion ::getsockopt().")
    if (umock::sys_socket_h.getsockopt(a_sfd, SOL_SOCKET, SO_ERROR,
                                       reinterpret_cast<char*>(&so_option),
                                       &optlen) != 0)
        throw_error("MSG1014: Failed to create socket:");

    m_sfd = a_sfd;
}

// Destructor
CSocket_basic::~CSocket_basic(){
    TRACE2(this, " Destruct CSocket_basic()") //
}

// Get the raw socket file descriptor
CSocket_basic::operator const SOCKET&() const {
    TRACE2(this, " Executing CSocket_basic::operator SOCKET&() (get "
                 "raw socket fd)")
    // There is no problem with cast here. We cast to const so we can only read.
    return const_cast<SOCKET&>(m_sfd);
}

// Getter
// ------
sa_family_t CSocket_basic::get_family() const {
    TRACE2(this, " Executing CSocket_basic::get_family()")
    m_get_addr_from_socket(__LINE__);
    return this->ss.ss_family;
}

const std::string& CSocket_basic::get_addr_str() {
    TRACE2(this, " Executing CSocket_basic::get_addr_str()")
    m_get_addr_from_socket(__LINE__);
    return SSockaddr::get_addr_str();
}

const std::string& CSocket_basic::get_addrp_str() {
    TRACE2(this, " Executing CSocket_basic::get_addrp_str()")
    m_get_addr_from_socket(__LINE__);
    return SSockaddr::get_addrp_str();
}

in_port_t CSocket_basic::get_port() const {
    TRACE2(this, " Executing CSocket_basic::get_port()")
    m_get_addr_from_socket(__LINE__);
    return SSockaddr::get_port();
}

int CSocket_basic::get_socktype() const {
    TRACE2(this, " Executing CSocket_basic::get_socktype()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_TYPE,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0)
        throw_error("MSG1030: Failed to get socket option SO_TYPE:");

    return so_option;
}

int CSocket_basic::get_sockerr() const {
    TRACE2(this, " Executing CSocket_basic::get_sockerr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_ERROR,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0)
        throw_error("MSG1011: Failed to get socket option SO_ERROR:");

    return so_option;
}

bool CSocket_basic::is_reuse_addr() const {
    TRACE2(this, " Executing CSocket_basic::is_reuse_addr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_REUSEADDR,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0)
        throw_error("MSG1013: Failed to get socket option SO_REUSEADDR:");

    return so_option;
}

bool CSocket_basic::is_bound() {
    // We get the socket address from the file descriptor and check if its
    // address is unspecified or if the address and port are all zero. This
    // replaces the method with direct compare of the socket addresses with a
    // 16 byte AF_INET6 compare which last version can be found at commit
    // a5ec86a93608234016630123c776c09f8ff276fb.
    TRACE2(this, " Executing CSocket::is_bound()")

    // binding is protected.
    std::scoped_lock lock(m_bound_mutex);

    const std::string& netaddr = this->get_addrp_str();
    return (netaddr.empty() || netaddr.compare("[::]:0") == 0 ||
            netaddr.compare("0.0.0.0:0") == 0)
               ? false
               : true;
}

// Private methods
// ---------------
void CSocket_basic::m_get_addr_from_socket(int line) const {
    TRACE2(this, " Executing CSocket_basic::m_get_addr_from_socket()")

    // Get address from socket file descriptor and fill in the inherited
    // sockaddr structure from SSockaddr.
    socklen_t len = this->sizeof_ss(); // May be modified
    if (upnplib::getsockname(m_sfd, &this->sa, &len) != 0)
        throw_error("MSG1001: [" + std::to_string(line) +
                    "] Failed to get address from socket:");
}


// CSocket class
// =============
// Default constructor for an empty socket object
CSocket::CSocket(){
    TRACE2(this, " Construct default CSocket()") //
}

// Constructor for new socket file descriptor
CSocket::CSocket(sa_family_t a_family, int a_socktype) {
    TRACE2(this, " Construct CSocket(af, socktype)")

    if (a_family != AF_INET6 && a_family != AF_INET)
        throw std::invalid_argument(
            UPNPLIB_LOGEXCEPT +
            "MSG1015: Failed to create socket: invalid address family " +
            std::to_string(a_family));
    if (a_socktype != SOCK_STREAM && a_socktype != SOCK_DGRAM)
        throw std::invalid_argument(
            UPNPLIB_LOGEXCEPT +
            "MSG1016: Failed to create socket: invalid socket type " +
            std::to_string(a_socktype));

    // Get new socket file descriptor.
    SOCKET sfd = ::socket(a_family, a_socktype, 0);
    if (sfd == INVALID_SOCKET)
        throw_error("MSG1017: Failed to create socket:");

    int so_option{0};
    constexpr socklen_t optlen{sizeof(so_option)};

    // Reset SO_REUSEADDR on all platforms if it should be set by default. This
    // is unclear on WIN32. See note below.
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<char*>(&so_option), optlen) != 0) {
        CLOSE_SOCKET_P(sfd);
        throw_error("MSG1018: Failed to set socket option SO_REUSEADDR:");
    }

#ifdef _MSC_VER
    // Set socket option SO_EXCLUSIVEADDRUSE on Microsoft Windows.
    // THIS IS AN IMPORTANT SECURITY ISSUE! Lock at
    // REF: [Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE]
    // (https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse#application-strategies)
    so_option = 1; // Set SO_EXCLUSIVEADDRUSE
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::setsockopt(sfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                     reinterpret_cast<char*>(&so_option), optlen) != 0) {
        CLOSE_SOCKET_P(sfd);
        throw_error(
            "MSG1019: Failed to set socket option SO_EXCLUSIVEADDRUSE:");
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
        if (::setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                         reinterpret_cast<char*>(&so_option), optlen) != 0) {
            CLOSE_SOCKET_P(sfd);
            throw_error("MSG1020: Failed to set socket option IPV6_V6ONLY:");
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
    if (::setsockopt(m_listen_sfd, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<char*>(&so_option), optlen) != 0)
        throw_error("MSG1004: Failed to set socket option SO_REUSEADDR:");
}
#endif

// Bind socket to local address
// REF:_[Bind:_Address_Already_in_Use]_(https://hea-www.harvard.edu/~fine/Tech/addrinuse.html)
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
        if (::setsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                         reinterpret_cast<const char*>(&so_option),
                         sizeof(so_option)) != 0)
            throw_error("MSG1007: Failed to set socket option IPV6_V6ONLY:");
    }

    // Here we bind the socket to an address
    const CAddrinfo ai(a_node, a_port, addr_family, this->get_socktype(),
                       AI_NUMERICHOST | AI_NUMERICSERV | a_flags);
    // Type cast socklen_t is needed for Microsoft Windows.
    if (::bind(m_sfd, ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen)) != 0)
        throw_error("MSG1008: Failed to bind socket to an address:");
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
        throw_error("MSG1034: Failed to set socket to listen:");

    m_listen = true;
}

// Set IPV6_V6ONLY
void CSocket::set_v6only(const bool a_opt) {
    TRACE2(this, " Executing CSocket::set_ipv6_v6only()")

    // Needed to have a valid argument for setsockopt()
    const int so_option{a_opt};

    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (this->get_family() == AF_INET6 && !this->is_bound() &&
        ::setsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                     reinterpret_cast<const char*>(&so_option),
                     sizeof(so_option)) != 0)
        throw_error("MSG1006: Failed to set socket option IPV6_V6ONLY:");
}

// Getter
// ------
bool CSocket::is_v6only() const {
    TRACE2(this, " Executing CSocket::is_v6only()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1028: Failed to get socket option "
                                 "IPV6_V6ONLY: \"Bad file descriptor\"");

    int so_option{0};
    socklen_t len{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    umock::sys_socket_h.getsockopt(m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                                   reinterpret_cast<char*>(&so_option), &len);

    return so_option;
}

bool CSocket::is_listen() const {
    TRACE2(this, " Executing CSocket::is_listen()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1035: Failed to get socket option "
                                 "'is_Listen': \"Bad file descriptor\"");

    // m_listen is protected.
    std::scoped_lock lock(m_listen_mutex);
    return m_listen;
}


// Portable handling of socket errors
// ==================================
CSocketError::CSocketError(){TRACE2(this, " Construct CSocketError()")}

CSocketError::~CSocketError(){TRACE2(this, " Destruct CSocketError()")}

CSocketError::operator const int&() const {
    // TRACE not usable with chained output.
    // TRACE2(this,
    //     " Executing CSocketError::operator int&() (get socket error number)")
    return m_errno;
}

void CSocketError::catch_error() {
#ifdef _MSC_VER
    m_errno = ::WSAGetLastError();
#else
    m_errno = errno;
#endif
    TRACE2(this, " Executing CSocketError::catch_error()")
}

std::string CSocketError::get_error_str() {
    // TRACE not usable with chained output, e.g.
    // std::cerr << "Error: " << sockerrObj.get_error_str();
    // TRACE2(this, " Executing CSocketError::get_error_str()")
#ifdef _MSC_VER
    return std::system_category().message(m_errno);
#else
    // return std::generic_category().message(m_errno);
    return std::strerror(m_errno);
#endif
}

} // namespace upnplib
