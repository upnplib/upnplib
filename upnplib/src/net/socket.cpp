// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-18

#include <upnplib/socket.hpp>
#include <upnplib/port.hpp>
#include <umock/sys_socket.hpp>

#include <string>
#include <cstring>
#include <stdexcept>

namespace upnplib {

#ifdef _MSC_VER
CWSAStartup::CWSAStartup() {
    TRACE2(this, " Construct upnplib::CWSAStartup")
    WSADATA wsaData;
    int rc = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        throw std::runtime_error("ERROR! Failed to initialize Windows "
                                 "sockets: WSAStartup() returns " +
                                 std::to_string(rc));
    }
}

CWSAStartup::~CWSAStartup() {
    TRACE2(this, " Destruct upnplib::CWSAStartup")
    ::WSACleanup();
}
#endif // MSC_VER


static inline void throw_error(std::string errmsg) {
    // error number given by WSAGetLastError(), resp. contained in errno is
    // used to specify details of the error. It is important that these error
    // numbers hasn't been modified by executing other statements.
#ifdef _MSC_VER
    throw std::runtime_error(
        errmsg + " WSAGetLastError()=" + std::to_string(WSAGetLastError()));
#else
    throw std::runtime_error(errmsg + " errno(" + std::to_string(errno) +
                             ")=\"" + std::strerror(errno) + "\"");
#endif
}

// Wrap socket() system call
// -------------------------
// Constructor for new socket file descriptor
CSocket::CSocket(int a_domain, int a_type, int a_protocol) {
    TRACE2(this, " Construct upnplib::CSocket()")

    // Check if we want an empty socket object.
    if (a_domain == 0 && a_type == 0 && a_protocol == 0)
        return;

    // Get socket file descriptor.
    SOCKET sfd = ::socket(a_domain, a_type, a_protocol);
    if (sfd == INVALID_SOCKET)
        throw_error("ERROR! Failed to create socket:");

    int so_option{0};
    constexpr socklen_t optlen{sizeof(so_option)};

    // Reset SO_REUSEADDR on all platforms if it should be set by default. This
    // is unclear on WIN32. See note below. If needed this option can be set
    // later with a setter.
    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&so_option,
                     optlen) != 0) {
        CLOSE_SOCKET_P(sfd);
        throw_error("ERROR! Failed to set socket option SO_REUSEADDR:");
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
        throw_error("ERROR! Failed to set socket option SO_EXCLUSIVEADDRUSE:");
    }
#endif

    // Set socket option IPV6_V6ONLY to false, means allowing IPv4 and IPv6.
    if (a_domain == AF_INET6) {
        so_option = 0;
        // Type cast (char*)&so_option is needed for Microsoft Windows.
        if (::setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&so_option,
                         optlen) != 0) {
            CLOSE_SOCKET_P(sfd);
            throw_error("ERROR! Failed to set socket option IPV6_V6ONLY:");
        }
    }

    // Store socket file descriptor and settings
    m_sfd = sfd;
    m_af = a_domain;
}

// Constructor with file desciptor
CSocket::CSocket([[maybe_unused]] SOCKET a_sfd) {}

// Move constructor
CSocket::CSocket(CSocket&& that) {
    TRACE2(this, " Construct move upnplib::CSocket()")
    m_af = that.m_af;
    that.m_af = -1;
    m_sfd = that.m_sfd;
    that.m_sfd = INVALID_SOCKET;

    // Following variables are protected
    std::scoped_lock lock(m_bound_mutex, m_listen_mutex);
    m_bound = that.m_bound;
    that.m_bound = false;
    m_listen = that.m_listen;
    that.m_listen = false;
}

// Assignment operator (parameter as value)
CSocket& CSocket::operator=(CSocket that) {
    TRACE2(this, " Executing upnplib::CSocket::operator=()")
    std::swap(m_sfd, that.m_sfd);
    std::swap(m_af, that.m_af);

    // Following variables are protected
    std::scoped_lock lock(m_bound_mutex, m_listen_mutex);
    std::swap(m_bound, that.m_bound);
    std::swap(m_listen, that.m_listen);

    return *this;
}

// Destructor
CSocket::~CSocket() {
    TRACE2(this, " Destruct upnplib::CSocket()")
    ::shutdown(m_sfd, SHUT_RDWR);
    CLOSE_SOCKET_P(m_sfd);
}

// Get the raw socket file descriptor
CSocket::operator SOCKET&() const {
    // There is no problem with cast here. We cast to const so we can only read.
    return const_cast<SOCKET&>(m_sfd);
}

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

// Setter: bind socket to local address
// REF: [Bind: Address Already in Use]
// (https://hea-www.harvard.edu/~fine/Tech/addrinuse.html)
void CSocket::bind(const CAddrinfo& a_ai) {
    TRACE2(this, " Executing upnplib::CSocket::bind()")

    int so_option{-1};
    socklen_t optlen{sizeof(so_option)}; // May be modified
    if (::getsockopt(m_sfd, SOL_SOCKET, SO_TYPE, (char*)&so_option, &optlen) !=
        0)
        throw_error("ERROR! Failed to bind socket to an address:");

    if (a_ai->ai_socktype != so_option)
        throw std::runtime_error("ERROR! Failed to bind socket to an address: "
                                 "\"socket type of address (" +
                                 std::to_string(a_ai->ai_socktype) +
                                 ") does not match socket type (" +
                                 std::to_string(so_option) + ")\"");
#ifdef _MSC_VER
    // There is a problem with binding on Microsoft Windows. It may be a delay
    // after freeing a bind even with different socket addresses. So we take
    // effort with trying some times to bind with respect to only short
    // locking. If the first attempt to bind succeeds there is no delay.
    constexpr int delay_inc{100};
    constexpr int delay_max{500}; // must be multiple of delay_inc.
    int i = delay_inc;
    for (; i <= delay_max; i += delay_inc) {
        TRACE2("Call STL function ::bind(), next delay = millisec ",
               std::to_string(i)) { // Only within this scope protect binding
                                    // and storing its state (m_bound).
            std::scoped_lock lock(m_bound_mutex);

            if (::bind(m_sfd, a_ai->ai_addr, (socklen_t)a_ai->ai_addrlen) ==
                0) {
                m_bound = true;
                break;
            }
        } // delay is not locked.
        std::this_thread::sleep_for(std::chrono::milliseconds(i));
    }
    if (i > delay_max)
        throw_error("ERROR! Failed to bind socket to an address:");

#else

    // Protect binding and storing its state (m_bound).
    std::scoped_lock lock(m_bound_mutex);

    if (::bind(m_sfd, a_ai->ai_addr, (socklen_t)a_ai->ai_addrlen) != 0)
        throw_error("ERROR! Failed to bind socket to an address:");

    m_bound = true;
#endif
}

// Setter: set socket to listen
void CSocket::listen() {
    TRACE2(this, " Executing upnplib::CSocket::listen()")

    // Protect set listen and storing its state (m_listen).
    std::scoped_lock lock(m_listen_mutex);

    // Second argument backlog (maximum length of the queue for pending
    // connections) is hard coded set to 1 for now.
    if (::listen(m_sfd, 1) != 0)
        throw_error("ERROR! Failed to set socket to listen:");

    m_listen = true;
}

// Getter
std::string CSocket::get_addr_str() const {
    TRACE2(this, " Executing upnplib::CSocket::get_addr_str()")
    sockaddr_storage ss{};
    this->get_sockname(&ss);

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
        throw std::invalid_argument(
            "ERROR! Failed to get a socket address string (IP "
            "address): unknown address family " +
            std::to_string(ss.ss_family));
    }

    if (ret == nullptr)
        throw_error(
            "ERROR! Failed to get a socket address string (IP address):");

    // Surround IPv6 address with '[' and ']'
    return (ss.ss_family == AF_INET6) ? '[' + std::string(addr_buf) + ']'
                                      : std::string(addr_buf);
}

uint16_t CSocket::get_port() const {
    TRACE2(this, " Executing upnplib::CSocket::get_port()")
    sockaddr_storage ss{};
    this->get_sockname(&ss);
    return ntohs(((sockaddr_in6*)&ss)->sin6_port);
}

int CSocket::get_sockerr() const {
    TRACE2(this, " Executing upnplib::CSocket::get_sockerr()")
    return this->get_sockopt_int(SOL_SOCKET, SO_ERROR, "SO_ERROR");
}

bool CSocket::is_reuse_addr() const {
    TRACE2(this, " Executing upnplib::CSocket::is_reuse_addr()")
    return this->get_sockopt_int(SOL_SOCKET, SO_REUSEADDR, "SO_REUSEADDR");
}

bool CSocket::is_v6only() const {
    TRACE2(this, " Executing upnplib::CSocket::is_v6only()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("ERROR! Failed to get socket option "
                                 "'is_v6only': \"Bad file descriptor\"");
    // We can have v6only with AF_INET6. Otherwise always false is returned.
    return (m_af == AF_INET6)
               ? this->get_sockopt_int(IPPROTO_IPV6, IPV6_V6ONLY, "IPV6_V6ONLY")
               : false;
}

bool CSocket::is_bind() const {
    TRACE2(this, " Executing upnplib::CSocket::is_bind()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("ERROR! Failed to get socket option "
                                 "'is_bind': \"Bad file descriptor\"");

    // m_bound is protected.
    std::scoped_lock lock(m_bound_mutex);
    return m_bound;
}

bool CSocket::is_listen() const {
    TRACE2(this, " Executing upnplib::CSocket::is_listen()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("ERROR! Failed to get socket option "
                                 "'is_Listen': \"Bad file descriptor\"");

    // m_listen is protected.
    std::scoped_lock lock(m_listen_mutex);
    return m_listen;
}

int CSocket::get_sockopt_int(int a_level, int a_optname,
                             const std::string& a_optname_str) const {
    TRACE2(this,
           " Executing upnplib::CSocket::get_sockopt_int(), " + a_optname_str)
    int so_option{-1};
    socklen_t optlen{sizeof(so_option)}; // May be modified

    // Type cast (char*)&so_option is needed for Microsoft Windows.
    if (::getsockopt(m_sfd, a_level, a_optname, (char*)&so_option, &optlen) !=
        0)
        throw_error("ERROR! Failed to get socket option " + a_optname_str +
                    ":");

    return so_option;
}

void CSocket::get_sockname(sockaddr_storage* a_ss) const {
    TRACE2(this, " Executing upnplib::CSocket::get_sockname()")
    const std::string errmsg{"ERROR! Failed to get socket address/port:"};

    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(errmsg + " \"Bad file descriptor\"");
    if (!this->is_bind())
        throw std::runtime_error(errmsg + " \"not bound to an address\"");
    // Get socket address
    socklen_t len = sizeof(sockaddr_storage); // May be modified
    if (umock::sys_socket_h.getsockname(m_sfd, (sockaddr*)a_ss, &len) != 0)
        throw_error(errmsg);
}

} // namespace upnplib
