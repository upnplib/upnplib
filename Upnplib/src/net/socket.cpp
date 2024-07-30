// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-07-30
/*!
 * \file
 * \brief Definition of the 'class Socket'.
 */

#include <upnplib/socket.hpp>

#include <upnplib/addrinfo.hpp>
#include <umock/sys_socket.hpp>
#include <umock/stringh.hpp>
#ifdef _MSC_VER
#include <umock/winsock2.hpp>
#endif

namespace upnplib {

namespace {

// Free helper functions
// =====================
/*!
 * \brief Wrapper for the ::%getsockname() system function
 * <!--   ------------------------------------------------ -->
 * \ingroup upnplib-socket
 *
 * The system function ::%getsockname() behaves different on different
 * platforms in particular with error handling on Microsoft Windows. This
 * function provides a portable version. The calling options are the same as
 * documented for the system function.
 */
int getsockname(SOCKET a_sockfd, sockaddr* a_addr, socklen_t* a_addrlen) {
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

} // anonymous namespace


// CSocket_basic class
// ===================
// Default constructor for an empty socket object
CSocket_basic::CSocket_basic(){
    TRACE2(this, " Construct default CSocket_basic()") //
}

// Constructor for the socket file descriptor. Before use, it must be load().
CSocket_basic::CSocket_basic(SOCKET a_sfd)
    : m_sfd_hint(a_sfd) {
    TRACE2(this, " Construct CSocket_basic(SOCKET)") //
}

// Setter with given file desciptor
void CSocket_basic::load() {
    TRACE2(this, " Executing CSocket_basic::load()")

    CSocketErr serrObj;
    // Check if we have a valid socket file descriptor
    int so_option{-1};
    socklen_t optlen{sizeof(so_option)}; // May be modified
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    TRACE2(this, " Calling system function ::getsockopt().")
    if (umock::sys_socket_h.getsockopt(m_sfd_hint, SOL_SOCKET, SO_ERROR,
                                       reinterpret_cast<char*>(&so_option),
                                       &optlen) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT + "MSG1014: Failed to create socket=" +
            std::to_string(m_sfd_hint) + ": " + serrObj.error_str() + "\n");
    }
    m_sfd = m_sfd_hint;
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
    m_get_addr_from_socket();
    return this->ss.ss_family;
}

const std::string& CSocket_basic::netaddr() {
    TRACE2(this, " Executing CSocket_basic::netaddr()")
    m_get_addr_from_socket();
    return SSockaddr::netaddr();
}

const std::string& CSocket_basic::netaddrp() {
    TRACE2(this, " Executing CSocket_basic::netaddrp()")
    m_get_addr_from_socket();
    return SSockaddr::netaddrp();
}

in_port_t CSocket_basic::get_port() const {
    TRACE2(this, " Executing CSocket_basic::get_port()")
    m_get_addr_from_socket();
    return SSockaddr::get_port();
}

int CSocket_basic::get_socktype() const {
    TRACE2(this, " Executing CSocket_basic::get_socktype()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    CSocketErr serrObj;
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_TYPE,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1030: Failed to get socket option SO_TYPE "
                                 "(SOCK_STREAM, or SOCK_DGRAM): " +
                                 serrObj.error_str() + "\n");
    }
    return so_option;
}

int CSocket_basic::get_sockerr() const {
    TRACE2(this, " Executing CSocket_basic::get_sockerr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    CSocketErr serrObj;
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_ERROR,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1011: Failed to get socket option SO_ERROR: " +
            serrObj.error_str() + "\n");
    }
    return so_option;
}

bool CSocket_basic::is_reuse_addr() const {
    TRACE2(this, " Executing CSocket_basic::is_reuse_addr()")
    int so_option{-1};
    socklen_t len{sizeof(so_option)}; // May be modified
    CSocketErr serrObj;
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.getsockopt(m_sfd, SOL_SOCKET, SO_REUSEADDR,
                                       reinterpret_cast<char*>(&so_option),
                                       &len) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1013: Failed to get socket option SO_REUSEADDR: " +
            serrObj.error_str() + "\n");
    }
    return so_option;
}

bool CSocket_basic::is_bound() {
    // I get the socket address from the file descriptor and check if its
    // address is unspecified or if the address and port are all zero. This
    // replaces the method with direct compare of the socket addresses with a
    // 16 byte AF_INET6 compare which last version can be found at commit
    // a5ec86a93608234016630123c776c09f8ff276fb.
    TRACE2(this, " Executing CSocket::is_bound()")

    // binding is protected.
    std::scoped_lock lock(m_bound_mutex);

    const std::string& netaddr = this->netaddrp();
    return (netaddr.empty() || netaddr.compare("[::]:0") == 0 ||
            netaddr.compare("0.0.0.0:0") == 0)
               ? false
               : true;
}

// Private methods
// ---------------
void CSocket_basic::m_get_addr_from_socket() const {
    TRACE2(this, " Executing CSocket_basic::m_get_addr_from_socket()")

    // Get address from socket file descriptor and fill in the inherited
    // sockaddr structure from SSockaddr.
    socklen_t len = this->sizeof_ss(); // May be modified
    CSocketErr serrObj;
    if (upnplib::getsockname(m_sfd, &this->sa, &len) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT + "MSG1001: Failed to get address from socket: " +
            serrObj.error_str() + "\n");
    }
}


// CSocket class
// =============
// Default constructor for an empty socket object
CSocket::CSocket(){
    TRACE2(this, " Construct default CSocket()") //
}

// clang-format off
// \brief Constructor for a new socket file descriptor that must be load()
CSocket::CSocket(sa_family_t a_family, int a_socktype)
    : m_pf_hint(a_family), m_socktype_hint(a_socktype) {
    TRACE2(this, " Construct CSocket() for socket fd") //
}

// Move constructor
CSocket::CSocket(CSocket && that) {
    TRACE2(this, " Construct move CSocket()")
    m_sfd = that.m_sfd;
    that.m_sfd = INVALID_SOCKET;

    // Following variables are protected
    std::scoped_lock lock(m_listen_mutex);
    m_listen = that.m_listen;
    that.m_listen = false;
}
// clang-format on

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
    TRACE2(this,
           " Destruct CSocket(), shutdown and close socket file descriptor")
    ::shutdown(m_sfd, SHUT_RDWR);
    CLOSE_SOCKET_P(m_sfd);
}

// Setter
// ------
// Setter to initialize the object with the hints given by the constructor
void CSocket::load() {
    TRACE2(this, " Executing CSocket::load(af, socktype)")

    // Do some general checks that must always be fulfilled according to the
    // specification.
    if (m_pf_hint != PF_INET6 && m_pf_hint != PF_INET)
        throw std::invalid_argument(
            UPNPLIB_LOGEXCEPT +
            "MSG1015: Failed to create socket: invalid protocol family " +
            std::to_string(m_pf_hint) + "\n");
    if (m_socktype_hint != SOCK_STREAM && m_socktype_hint != SOCK_DGRAM)
        throw std::invalid_argument(
            UPNPLIB_LOGEXCEPT +
            "MSG1016: Failed to create socket: invalid socket type " +
            std::to_string(m_socktype_hint) + "\n");

    // Do nothing if there is already a valid socket file descriptor from a
    // previous load().
    if (m_sfd != INVALID_SOCKET)
        return;

    CSocketErr serrObj;

    // Syscall socket(): get new socket file descriptor.
    SOCKET sfd = umock::sys_socket_h.socket(m_pf_hint, m_socktype_hint, 0);
    if (sfd == INVALID_SOCKET) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1017: Failed to create socket: " + serrObj.error_str() + "\n");
    }
    int so_option{0};
    constexpr socklen_t optlen{sizeof(so_option)};

    // Reset SO_REUSEADDR on all platforms if it should be set by default. This
    // is unclear on WIN32. See note below.
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                                       reinterpret_cast<char*>(&so_option),
                                       optlen) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1018: Failed to set socket option SO_REUSEADDR: " +
            serrObj.error_str() + "\n");
    }

#ifdef _MSC_VER
    // Set socket option SO_EXCLUSIVEADDRUSE on Microsoft Windows.
    // THIS IS AN IMPORTANT SECURITY ISSUE! Lock at
    // REF: [Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE]
    // (https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse#application-strategies)
    so_option = 1; // Set SO_EXCLUSIVEADDRUSE
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (umock::sys_socket_h.setsockopt(sfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                                       reinterpret_cast<char*>(&so_option),
                                       optlen) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1019: Failed to set socket option SO_EXCLUSIVEADDRUSE: " +
            serrObj.error_str() + "\n");
    }
#endif

    // Store socket file descriptor
    m_sfd = sfd;
}

// Set IPV6_V6ONLY
void CSocket::set_v6only(const bool a_opt) {
    TRACE2(this, " Executing CSocket::set_ipv6_v6only()")
    CSocketErr serrObj;

    // Needed to have a valid argument for setsockopt()
    const int so_option{a_opt};

    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (this->get_family() == PF_INET6 && !this->is_bound() &&
        umock::sys_socket_h.setsockopt(
            m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
            reinterpret_cast<const char*>(&so_option),
            sizeof(so_option)) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1006: Failed to set socket option IPV6_V6ONLY: " +
            serrObj.error_str() + "\n");
    }
}

#if 0
void CSocket::set_reuse_addr(bool a_reuse) {
    // Set socket option SO_REUSEADDR on other platforms.
    // --------------------------------------------------
    // REF: [How do SO_REUSEADDR and SO_REUSEPORT differ?]
    // (https://stackoverflow.com/a/14388707/5014688)
    so_option = a_reuse_addr ? 1 : 0;
    // Type cast (char*)&so_reuseaddr is needed for Microsoft Windows.
    if (umock::sys_socket_h.setsockopt(m_listen_sfd, SOL_SOCKET, SO_REUSEADDR,
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
    CSocketErr serrObj;

    // With protocol family PF_INET6 we always set IPV6_V6ONLY to true. See also
    // note to bind() in the header file.
    if (addr_family == PF_INET6) {
        // Don't use 'this->set_v6only(true)' because binding is protected with
        // a mutex and we will get a deadlock due to using
        // 'this->is_bound()' in 'this->set_v6only(true)'.
        constexpr int so_option{1}; // true
        // Type cast (char*)&so_option is needed for Microsoft Windows.
        if (umock::sys_socket_h.setsockopt(
                m_sfd, IPPROTO_IPV6, IPV6_V6ONLY,
                reinterpret_cast<const char*>(&so_option),
                sizeof(so_option)) != 0) {
            serrObj.catch_error();
            throw std::runtime_error(
                UPNPLIB_LOGEXCEPT +
                "MSG1007: Failed to set socket option IPV6_V6ONLY: " +
                serrObj.error_str() + "\n");
        }
    }

    // Get an adress info to bind.
    CAddrinfo ai(a_node, a_port, addr_family, this->get_socktype(),
                 AI_NUMERICHOST | AI_NUMERICSERV | a_flags);
    ai.load(); // may throw exception

    // Here we bind the socket to an address.
    /// \todo Improve CSocketErr for specific ::bind() error messages.
    // Type cast socklen_t is needed for Microsoft Windows.
    int ret = umock::sys_socket_h.bind(m_sfd, ai->ai_addr,
                                       static_cast<socklen_t>(ai->ai_addrlen));

    UPNPLIB_LOGINFO << "MSG1115: syscall ::bind(" << m_sfd << ", "
                    << ai->ai_addr << ", " << ai->ai_addrlen << ") Using \""
                    << ai.netaddr().str() << "\". Get "
                    << (ret != 0 ? "ERROR" : this->netaddrp()) << "\n";
    if (ret != 0) {
        serrObj.catch_error();
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT +
            "MSG1008: Failed to bind socket to an address: " +
            serrObj.error_str() + "\n");
    }
}

// Set socket to listen
void CSocket::listen() {
    TRACE2(this, " Executing CSocket::listen()")

    // Protect set listen and storing its state (m_listen).
    std::scoped_lock lock(m_listen_mutex);

    if (m_listen)
        return;

    CSocketErr serrObj;
    // Second argument backlog (maximum length of the queue for pending
    // connections) is hard coded set to 1 for now.
    if (umock::sys_socket_h.listen(m_sfd, 1) != 0) {
        serrObj.catch_error();
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1034: Failed to set socket to listen: " +
                                 serrObj.error_str() + "\n");
    }

    m_listen = true;
}

// Getter
// ------
bool CSocket::is_v6only() const {
    TRACE2(this, " Executing CSocket::is_v6only()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(UPNPLIB_LOGEXCEPT +
                                 "MSG1028: Failed to get socket option "
                                 "IPV6_V6ONLY: \"Bad file descriptor\"\n");

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
                                 "'is_Listen': \"Bad file descriptor\"\n");

    // m_listen is protected.
    std::scoped_lock lock(m_listen_mutex);
    return m_listen;
}


// Portable handling of socket errors
// ==================================
CSocketErr::CSocketErr() = default;

CSocketErr::~CSocketErr() = default;

CSocketErr::operator const int&() {
    // TRACE not usable with chained output.
    // TRACE2(this, " Executing CSocketErr::operator int&() (get socket error
    // number)")
    return m_errno;
}

void CSocketErr::catch_error() {
#ifdef _MSC_VER
    m_errno = umock::winsock2_h.WSAGetLastError();
#else
    m_errno = errno;
#endif
    TRACE2(this, " Executing CSocketErr::catch_error()")
}

std::string CSocketErr::error_str() const {
    // TRACE not usable with chained output, e.g.
    // std::cerr << "Error: " << sockerrObj.error_str();
    // TRACE2(this, " Executing CSocketErr::error_str()")

    // Portable C++ statement
    return std::system_category().message(m_errno);
    // return std::generic_category().message(m_errno);
    // return std::strerror(m_errno); // Alternative for Unix platforms
}

} // namespace upnplib
