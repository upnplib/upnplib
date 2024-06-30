// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-07-01
/*!
 * \file
 * \brief Definition of the Sockaddr class and some free helper functions.
 */

#include <upnplib/sockaddr.hpp>
#include <upnplib/global.hpp>
#include <upnplib/synclog.hpp>
#include <upnplib/netaddr.hpp>
#include <cstring>

namespace upnplib {

namespace {

// Free function to get the address string from a sockaddr structure
// -----------------------------------------------------------------
/*! \brief Get the [netaddress](\ref glossary_netaddr) without port from a
 * sockaddr structure
 * \ingroup upnplib-addrmodul
 * \code
 * // Usage e.g.:
 * ::sockaddr_storage saddr{};
 * std::cout << "netaddress is " << to_netaddr(&saddr) << "\n";
 * \endcode
 */
std::string to_netaddr(const ::sockaddr_storage* const a_sockaddr) noexcept {
    // TRACE("Executing to_netaddr()") // not usable in chained output.
    char addrbuf[INET6_ADDRSTRLEN]{};

    switch (a_sockaddr->ss_family) {
    case AF_UNSPEC:
        return "";

    // There is no need to test the return value of ::inet_ntop() because its
    // two possible errors are implicit managed by this code.
    case AF_INET6: {
        const ::sockaddr_in6* sin6 =
            reinterpret_cast<const ::sockaddr_in6*>(a_sockaddr);
        ::inet_ntop(AF_INET6, sin6->sin6_addr.s6_addr, addrbuf,
                    sizeof(addrbuf));
        // Next throws 'std::length_error' if the length of the constructed
        // string would exceed max_size(). This should never happen with given
        // length of addrbuf.
        if (sin6->sin6_scope_id > 0)
            return '[' + std::string(addrbuf) + '%' +
                   std::to_string(sin6->sin6_scope_id) + ']';
        return '[' + std::string(addrbuf) + ']';
    }
    case AF_INET:
        ::inet_ntop(AF_INET,
                    &reinterpret_cast<const ::sockaddr_in*>(a_sockaddr)
                         ->sin_addr.s_addr,
                    addrbuf, sizeof(addrbuf));
        // Next throws 'std::length_error' if the length of the constructed
        // string would exceed max_size(). This should never happen with given
        // length of addrbuf.
        return std::string(addrbuf);

    default:
        UPNPLIB_LOGERR "MSG1036: Unsupported address family "
            << static_cast<int>(a_sockaddr->ss_family) << ".\n";
    }
    return "";
}


// Free function to get the port number from a string
// --------------------------------------------------
/*! \brief Get the port number from a string
 * \ingroup upnplib-addrmodul
 * \code
 * ~$ // Usage e.g.:
 * ~$ in_port_t port = to_port("55555");
 * \endcode
 *
 * Checks if the given string represents a numeric value between 0 and 65535.
 * \returns
 *  On success: Value of the port number, an empty string returns 0.
 * \exception std::invalid_argument Invalid port number
 */
in_port_t to_port(const std::string& a_port_str) {
    TRACE("Executing SSockaddr::this->to_port() with port=\"" + a_port_str +
          "\"")
    if (a_port_str.empty())
        return 0;
    if (is_numport(a_port_str) == 0) {
        int port = std::stoi(a_port_str);
        return static_cast<in_port_t>(port);
    }
    throw std::invalid_argument(
        UPNPLIB_LOGEXCEPT + "MSG1033: Failed to get port number for string \"" +
        a_port_str + "\"");
}


// Free function to logical compare two sockaddr structures
// --------------------------------------------------------
/*! \brief logical compare two sockaddr structures
 * \ingroup upnplib-addrmodul
 *
 * To have a logical equal socket address we compare the address family, the ip
 * address and the port.
 *
 * \returns
 *  \b true if socket addresses are logical equal\n
 *  \b false otherwise
 */
bool sockaddrcmp(const ::sockaddr_storage* a_ss1,
                 const ::sockaddr_storage* a_ss2) noexcept {
    // To have a logical equal socket address we compare the address family, the
    // ip address and the port.
    // Throws no exception.
    if (a_ss1 == nullptr && a_ss2 == nullptr)
        return true;
    if (a_ss1 == nullptr || a_ss2 == nullptr)
        return false;

    switch (a_ss1->ss_family) {
    case AF_UNSPEC:
        if (a_ss2->ss_family != AF_UNSPEC)
            return false;
        break;

    case AF_INET6: {
        // We compare ipv6 addresses which are stored in a 16 byte array
        // (unsigned char s6_addr[16]). So we have to use memcmp() for
        // comparison.
        const unsigned char* const s6_addr1 =
            reinterpret_cast<const ::sockaddr_in6*>(a_ss1)->sin6_addr.s6_addr;
        const unsigned char* const s6_addr2 =
            reinterpret_cast<const ::sockaddr_in6*>(a_ss2)->sin6_addr.s6_addr;

        if (a_ss2->ss_family != AF_INET6 ||
            ::memcmp(s6_addr1, s6_addr2, sizeof(in6_addr)) != 0 ||
            reinterpret_cast<const ::sockaddr_in6*>(a_ss1)->sin6_port !=
                reinterpret_cast<const ::sockaddr_in6*>(a_ss2)->sin6_port)
            return false;
    } break;

    case AF_INET:
        if (a_ss2->ss_family != AF_INET ||
            reinterpret_cast<const ::sockaddr_in*>(a_ss1)->sin_addr.s_addr !=
                reinterpret_cast<const ::sockaddr_in*>(a_ss2)
                    ->sin_addr.s_addr ||
            reinterpret_cast<const ::sockaddr_in*>(a_ss1)->sin_port !=
                reinterpret_cast<const ::sockaddr_in*>(a_ss2)->sin_port)
            return false;
        break;

    default:
        return false;
    }

    return true;
}

} // anonymous namespace


// Free function to get the address string with port from a sockaddr structure
// ---------------------------------------------------------------------------
std::string to_netaddrp(const ::sockaddr_storage* const a_sockaddr) noexcept {
    // TRACE("Executing to_addrport_str()") // not usable in chained output.
    //
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so I can use it for AF_INET and AF_INET6.
    // 'std::to_string()' may throw 'std::bad_alloc' from the std::string
    // constructor. It is a fatal error that violates the promise to noexcpt
    // and immediately terminates the propgram. This is intentional because the
    // error cannot be handled.
    return (a_sockaddr->ss_family != AF_INET6 &&
            a_sockaddr->ss_family != AF_INET)
               ? to_netaddr(a_sockaddr) // let it handle the error.
               : to_netaddr(a_sockaddr) + ":" +
                     std::to_string(ntohs(
                         reinterpret_cast<const ::sockaddr_in6*>(a_sockaddr)
                             ->sin6_port));
}


// Specialized sockaddr_structure
// ==============================
// Constructor
SSockaddr::SSockaddr(){
    TRACE2(this, " Construct SSockaddr()") //
}

// Destructor
SSockaddr::~SSockaddr() {
    TRACE2(this, " Destruct SSockaddr()")
    // Destroy structure
    ::memset(&m_sa_union, 0xAA, sizeof(m_sa_union));
}

// Get reference to the sockaddr_storage structure.
// Only as example, we don't use it.
// SSockaddr::operator const ::sockaddr_storage&() const {
//     return this->ss;
// }

// Copy constructor
// ----------------
SSockaddr::SSockaddr(const SSockaddr& that) {
    TRACE2(this, " Construct copy SSockaddr()")
    m_sa_union = that.m_sa_union;
}

// Copy assignment operator
// ------------------------
SSockaddr& SSockaddr::operator=(SSockaddr that) {
    TRACE2(this,
           " Executing SSockaddr::operator=(SSockaddr) (struct assign op).")
    std::swap(m_sa_union, that.m_sa_union);

    return *this;
}

// Assignment operator= to set socket address from string,
// -------------------------------------------------------
// For port conversion:
// Don't use '::htons' (with colons) instead of 'htons', MacOS don't like it.
// 'sin6_port' is also 'sin_port' due to union.
void SSockaddr::operator=(const std::string& a_addr_str) {
    // Valid input examles: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                      "192.168.1.1", "192.168.1.1:50001".
    TRACE2(this, " Executing SSockaddr::operator=()")

    // An empty address string clears the address storage.
    if (a_addr_str.empty()) {
        ::memset(&m_sa_union, 0, sizeof(m_sa_union));
        return;
    }

    if (a_addr_str.front() == '[') { // IPv6 address
        if (a_addr_str.back() == ']') { // IPv6 address without port
            this->handle_ipv6(a_addr_str);

        } else { // IPv6 with port
            // Split address and port
            size_t pos = a_addr_str.find("]:");
            this->handle_ipv6(a_addr_str.substr(0, pos + 1));
            sin6.sin6_port = htons(to_port(a_addr_str.substr(pos + 2)));
        }
    } else { // IPv4 address or port
        size_t pos = a_addr_str.find_first_of(':');
        if (pos == 0 && a_addr_str.size() > 1 && ss.ss_family != AF_UNSPEC) {
            // Special case: only port with leading ':' is valid to change port
            // of existing socket address.
            sin6.sin6_port = htons(to_port(a_addr_str.substr(pos + 1)));

        } else if (pos != std::string::npos) { // ':' found means ipv4 with port
            this->handle_ipv4(a_addr_str.substr(0, pos));
            sin6.sin6_port = htons(to_port(a_addr_str.substr(pos + 1)));

        } else { // IPv4 without port or port only
            if (a_addr_str.find_first_of(".") != std::string::npos) {
                this->handle_ipv4(a_addr_str); // IPv4 address without port

            } else { // port only
                sin6.sin6_port = htons(to_port(a_addr_str));
            }
        }
    }
}

// Assignment operator= to set socket port from an integer,
// --------------------------------------------------------
void SSockaddr::operator=(const in_port_t a_port) {
    // Don't use ::htons, MacOS don't like it.
    // sin6_port is also sin_port due to union.
    sin6.sin6_port = htons(a_port);
}

// Compare operator== to test if another trivial socket address is equal to this
// -----------------------------------------------------------------------------
bool SSockaddr::operator==(const ::sockaddr_storage& a_ss) const {
    return sockaddrcmp(&a_ss, &ss);
}

// Getter for the assosiated ip address without port
// -------------------------------------------------
// e.g. "[2001:db8::2]" or "192.168.254.253".
const std::string& SSockaddr::netaddr() {
    // TRACE not usable with chained output.
    // TRACE2(this, " Executing SSockaddr::netaddr()")
    //
    // It is important to have the string available as long as the object lives,
    // otherwise you may get dangling pointer, e.g. with getting .c_str().
    m_netaddr = to_netaddr(&ss);
    return m_netaddr;
}

// Getter for the assosiated ip address with port
// ----------------------------------------------
// e.g. "[2001:db8::2]:50001" or "192.168.254.253:50001".
const std::string& SSockaddr::netaddrp() {
    // TRACE not usable with chained output.
    // TRACE2(this, " Executing SSockaddr::netaddrp()")
    //
    // It is important to have the string available as long as the object lives,
    // otherwise you may get dangling pointer, e.g. with getting .c_str().
    m_netaddrp = to_netaddrp(&ss);
    return m_netaddrp;
}

// Getter for the assosiated port number
// -------------------------------------
in_port_t SSockaddr::get_port() const {
    TRACE2(this, " Executing SSockaddr::get_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    // Don't use ::ntohs, MacOS don't like it.
    return ntohs(sin6.sin6_port);
}

// Getter for sizeof the Sockaddr Structure.
// -----------------------------------------
socklen_t SSockaddr::sizeof_ss() const {
    TRACE2(this, " Executing SSockaddr::sizeof_ss()")
    return sizeof(ss);
}

// Getter for sizeof the current (sin6 or sin) Sockaddr Structure.
// ---------------------------------------------------------------
socklen_t SSockaddr::sizeof_saddr() const {
    TRACE2(this, " Executing SSockaddr::sizeof_saddr()")
    switch (ss.ss_family) {
    case AF_INET6:
        return sizeof(sin6);
    case AF_INET:
        return sizeof(sin);
    default:
        return 0;
    }
}


// private member functions
// ------------------------
void SSockaddr::handle_ipv6(const std::string& a_addr_str) {
    TRACE2(this, " Executing SSockaddr::handle_ipv6()")
    // remove surounding brackets
    std::string addr_str = a_addr_str.substr(1, a_addr_str.length() - 2);

    int ret = ::inet_pton(AF_INET6, addr_str.c_str(), &sin6.sin6_addr);
    if (ret == 0) {
        throw std::invalid_argument(UPNPLIB_LOGEXCEPT +
                                    "MSG1043: Invalid netaddress \"" +
                                    a_addr_str + "\".");
    }
    ss.ss_family = AF_INET6;
}

void SSockaddr::handle_ipv4(const std::string& a_addr_str) {
    TRACE2(this, " Executing SSockaddr::handle_ipv4()")
    int ret = ::inet_pton(AF_INET, a_addr_str.c_str(), &sin.sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(UPNPLIB_LOGEXCEPT +
                                    "MSG1044: Invalid netaddress \"" +
                                    a_addr_str + "\".");
    }
    ss.ss_family = AF_INET;
}

} // namespace upnplib
