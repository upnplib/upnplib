// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-05-05

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
// Default constructor for an empty socket object
CSocket::CSocket(){TRACE2(this, " Construct default upnplib::CSocket()")}

// Constructor for new socket file descriptor
CSocket::CSocket(sa_family_t a_domain, int a_type, int a_protocol) {
    TRACE2(this, " Construct upnplib::CSocket() with address family and type")

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

// Constructor with given file desciptor
CSocket::CSocket(SOCKET a_sfd) {
    TRACE2(this, " Construct upnplib::CSocket(SOCKET)")

    // Get address family. We can only get it from a bound socket.
    // Otherwise we cache AF_UNSPEC.
    ::sockaddr_storage ss{};
    ss.ss_family = AF_UNSPEC;
    socklen_t len = sizeof(ss); // May be modified
    if (umock::sys_socket_h.getsockname(a_sfd, (sockaddr*)&ss, &len) != 0)
        throw_error("ERROR! Failed to get socket address/port:");

    m_sfd = a_sfd;
    m_af = ss.ss_family;
}

// Move constructor
CSocket::CSocket(CSocket&& that) {
    TRACE2(this, " Construct move upnplib::CSocket()")
    m_af = that.m_af;
    that.m_af = AF_UNSPEC;
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

    // Protect binding and storing its state (m_bound).
    std::scoped_lock lock(m_bound_mutex);

    int so_type = this->get_sockopt_int(SOL_SOCKET, SO_TYPE, "SO_TYPE");
    if (a_ai->ai_socktype != so_type)
        throw std::runtime_error("ERROR! Failed to bind socket to an address: "
                                 "\"socket type of address (" +
                                 std::to_string(a_ai->ai_socktype) +
                                 ") does not match socket type (" +
                                 std::to_string(so_type) + ")\"");

    if (::bind(m_sfd, a_ai->ai_addr, (socklen_t)a_ai->ai_addrlen) != 0)
        throw_error("ERROR! Failed to bind socket to an address:");

    m_bound = true;
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
    ::sockaddr_storage ss{};
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
    ::sockaddr_storage ss{};
    this->get_sockname(&ss);
    return ntohs(((sockaddr_in6*)&ss)->sin6_port);
}

sa_family_t CSocket::get_af() const {
    TRACE2(this, " Executing upnplib::CSocket::get_af()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("ERROR! Failed to get address family: "
                                 "\"Bad file descriptor\"");

    return m_af;
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
    // We assume that a socket with an unknown ip address and port 0 is unbound.
    TRACE2(this, " Executing upnplib::CSocket::is_bind()")
    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error("ERROR! Failed to get socket option "
                                 "'is_bind': \"Bad file descriptor\"");

    // binding is protected.
    std::scoped_lock lock(m_bound_mutex);

    ::sockaddr_storage ss{};
    this->get_sockname(&ss);

    switch (ss.ss_family) {
    case AF_UNSPEC:
        return false;
    case AF_INET6:
        if (((sockaddr_in6*)&ss)->sin6_port == 0 &&
            ((sockaddr_in6*)&ss)->sin6_port == 0)
            return false;
        break;
    case AF_INET:
        if (((sockaddr_in*)&ss)->sin_port == 0 &&
            ((sockaddr_in*)&ss)->sin_port == 0)
            return false;
        break;
    default:
        throw std::runtime_error("ERROR! Failed to get socket option "
                                 "'is_bind': unsupported address family " +
                                 std::to_string(ss.ss_family));
    }

    return true;
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

void CSocket::get_sockname(::sockaddr_storage* a_ss) const {
    TRACE2(this, " Executing upnplib::CSocket::get_sockname()")
    const std::string errmsg{"ERROR! Failed to get socket address/port:"};

    if (m_sfd == INVALID_SOCKET)
        throw std::runtime_error(errmsg + " \"Bad file descriptor\"");
    socklen_t len = sizeof(*a_ss); // May be modified
    if (umock::sys_socket_h.getsockname(m_sfd, (sockaddr*)a_ss, &len) != 0)
        throw_error(errmsg);
}

} // namespace upnplib
