// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-28

#include "upnplib/sockaddr.hpp"
#include "umock/sys_socket.hpp"
#include "umock/stringh.hpp"
#include <filesystem>
#include <cstring>
#include <string>


namespace upnplib {

// Free function to get the port number from a string
// --------------------------------------------------
uint16_t to_port(const std::string& a_port_str) {
    TRACE("Executing upnplib::to_port()")

    if (a_port_str.empty())
        return 0;

    int port;

    // Only strings with max. 5 characters are valid (uint16_t has max. 65535)
    if (a_port_str.length() > 5)
        goto throw_exit;

    // Now we check if the string are all digit characters
    for (unsigned char ch : a_port_str) {
        if (!isdigit(ch))
            goto throw_exit;
    }
    // Valid positive number but is it within the port range (uint16_t)?
    port = std::stoi(a_port_str);
    if (port <= 65535)

        return static_cast<uint16_t>(port);

throw_exit:
    throw std::invalid_argument("ERROR! Failed to get port number for \"" +
                                a_port_str + "\"");
}


// Specialized sockaddr_structure
// ------------------------------
// Constructor
Sockaddr_storage::Sockaddr_storage() {
    TRACE2(this, " Construct upnplib::Sockaddr_storage()") //
    memset(&this->ss, 0, sizeof(this->ss));
}

// Destructor
Sockaddr_storage::~Sockaddr_storage() {
    TRACE2(this, " Destruct upnplib::Sockaddr_storage()") //
}

void Sockaddr_storage::operator=(const std::string& a_addr_str) {
    // Input examples: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                 "192.168.1.1", "192.168.1.1:50001".
    TRACE2(this, " Executing upnplib::Sockaddr_storage::operator=()")

    if (a_addr_str.front() == '[') {    // IPv6 address
        if (a_addr_str.back() == ']') { // IPv6 address without port
            // remove surounding brackets
            m_addr_str = a_addr_str.substr(1, a_addr_str.length() - 2);
            this->handle_ipv6();

        } else { // IPv6 with port
            // Split address and port
            size_t pos = a_addr_str.find("]:");
            m_addr_str = a_addr_str.substr(1, pos - 1);
            m_port = to_port(a_addr_str.substr(pos + 2));
            this->handle_ipv6_with_port();
        }
    } else { // IPv4 address or port
        size_t pos = a_addr_str.find_first_of(":");
        if (pos != std::string::npos) { // ':' found means ipv4 with port
            m_addr_str = a_addr_str.substr(0, pos);
            m_port = to_port(a_addr_str.substr(pos + 1));
            this->handle_ipv4_with_port();

        } else { // IPv4 without port or port only
            if (a_addr_str.find_first_of(".") != std::string::npos) {
                m_addr_str = a_addr_str; // IPv4 address must be without port
                this->handle_ipv4();

            } else { // port only
                m_port = to_port(a_addr_str);
                this->handle_port();
            }
        }
    }
}

void Sockaddr_storage::handle_port() {
    TRACE2(this, " Executing upnplib::Sockaddr_storage::handle_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    ((sockaddr_in6*)&this->ss)->sin6_port = htons(m_port);
}

void Sockaddr_storage::handle_ipv6() {
    TRACE2(this, " Executing upnplib::Sockaddr_storage::handle_ipv6()")
    int ret = inet_pton(AF_INET6, m_addr_str.c_str(),
                        &((sockaddr_in6*)&this->ss)->sin6_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '[" +
            m_addr_str + "]'");
    }
    this->ss.ss_family = AF_INET6;
}

void Sockaddr_storage::handle_ipv6_with_port() {
    TRACE2(this,
           " Executing upnplib::Sockaddr_storage::handle_ipv6_with_port()")
    this->handle_ipv6();
    this->handle_port();
}

void Sockaddr_storage::handle_ipv4() {
    TRACE2(this, " Executing upnplib::Sockaddr_storage::handle_ipv4()")
    int ret = inet_pton(AF_INET, m_addr_str.c_str(),
                        &((sockaddr_in*)&this->ss)->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            m_addr_str + "'");
    }
    this->ss.ss_family = AF_INET;
}

void Sockaddr_storage::handle_ipv4_with_port() {
    TRACE2(this,
           " Executing upnplib::Sockaddr_storage::handle_ipv4_with_port()")
    this->handle_ipv4();
    this->handle_port();
}


std::string Sockaddr_storage::get_addr_str() const {
    TRACE2(this, " Executing upnplib::Sockaddr_storage::get_addr_str()")
    char addrbuf[INET6_ADDRSTRLEN]{};

    if (this->ss.ss_family == AF_INET6) {
        inet_ntop(AF_INET6, ((sockaddr_in6*)&this->ss)->sin6_addr.s6_addr,
                  addrbuf, sizeof(addrbuf));

        return '[' + std::string(addrbuf) + ']';

    } else {

        inet_ntop(this->ss.ss_family,
                  &((sockaddr_in*)&this->ss)->sin_addr.s_addr, addrbuf,
                  sizeof(addrbuf));
    }

    return std::string(addrbuf);
}

uint16_t Sockaddr_storage::get_port() const {
    TRACE2(this, " Executing upnplib::Sockaddr_storage::get_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    return ntohs(((sockaddr_in6*)&this->ss)->sin6_port);
}


#if false
// Specialized sockaddr_structure derived from system ::sockaddr_structure
// -----------------------------------------------------------------------
sockaddr_storage::sockaddr_storage() {
    TRACE2(this, " Construct upnplib::sockaddr_storage()")
    memset((::sockaddr_storage*)&this->ss_family, 0,
           sizeof(::sockaddr_storage));
}

sockaddr_storage::sockaddr_storage(const std::string& a_addr_str,
                                   uint16_t a_port) {
    TRACE2(this, " Construct upnplib::sockaddr_storage(addr_str, port)")
    int ret = inet_pton(this->ss_family, a_addr_str.c_str(),
                        &((sockaddr_in*)this)->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            a_addr_str + "'");
    }
    // sin_port and sin6_port are on the same destination.
    ((sockaddr_in6*)this)->sin6_port = htons(a_port);
}

sockaddr_storage::~sockaddr_storage() {
    TRACE2(this, " Destruct upnplib::sockaddr_storage()") //
}


void sockaddr_storage::operator=(const std::string& a_addr_str) {
    // Input examples: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                 "192.168.1.1", "192.168.1.1:50001".
    TRACE2(this, " Executing upnplib::sockaddr_storage::operator=()")

    if (a_addr_str.front() == '[') {    // IPv6 address
        if (a_addr_str.back() == ']') { // IPv6 address without port
            // remove surounding brackets
            m_addr_str = a_addr_str.substr(1, a_addr_str.length() - 2);
            this->handle_ipv6();

        } else { // IPv6 with port
            // Split address and port
            size_t pos = a_addr_str.find("]:");
            m_addr_str = a_addr_str.substr(1, pos - 1);
            m_port = to_port(a_addr_str.substr(pos + 2));
            this->handle_ipv6_with_port();
        }
    } else { // IPv4 address or port
        size_t pos = a_addr_str.find_first_of(":");
        if (pos != std::string::npos) { // ':' found means ipv4 with port
            m_addr_str = a_addr_str.substr(0, pos);
            m_port = to_port(a_addr_str.substr(pos + 1));
            this->handle_ipv4_with_port();

        } else { // IPv4 without port or port only
            if (a_addr_str.find_first_of(".") != std::string::npos) {
                m_addr_str = a_addr_str; // IPv4 address must be without port
                this->handle_ipv4();

            } else { // port only
                m_port = to_port(a_addr_str);
                this->handle_port();
            }
        }
    }
}

std::string sockaddr_storage::get_addr_str() const {
    TRACE2(this, " Executing upnplib::sockaddr_storage::get_addr_str()")
    char addrbuf[INET6_ADDRSTRLEN]{};

    if (this->ss_family == AF_INET6) {
        inet_ntop(AF_INET6, sin6->sin6_addr.s6_addr, addrbuf, sizeof(addrbuf));

        return '[' + std::string(addrbuf) + ']';

    } else {

        inet_ntop(this->ss_family, &sin->sin_addr.s_addr, addrbuf,
                  sizeof(addrbuf));
    }

    return std::string(addrbuf);
}

uint16_t sockaddr_storage::get_port() const {
    TRACE2(this, " Executing upnplib::sockaddr_storage::get_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    return ntohs(sin6->sin6_port);
}


void sockaddr_storage::handle_port() {
    TRACE2(this, " Executing upnplib::sockaddr_storage::handle_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    sin6->sin6_port = htons(m_port);
}

void sockaddr_storage::handle_ipv6() {
    TRACE2(this, " Executing upnplib::sockaddr_storage::handle_ipv6()")
    int ret = inet_pton(AF_INET6, m_addr_str.c_str(), &sin6->sin6_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '[" +
            m_addr_str + "]'");
    }
    this->ss_family = AF_INET6;
}

void sockaddr_storage::handle_ipv6_with_port() {
    TRACE2(this,
           " Executing upnplib::sockaddr_storage::handle_ipv6_with_port()")
    this->handle_ipv6();
    this->handle_port();
}

void sockaddr_storage::handle_ipv4() {
    TRACE2(this, " Executing upnplib::sockaddr_storage::handle_ipv4()")
    int ret = inet_pton(AF_INET, m_addr_str.c_str(), &sin->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            m_addr_str + "'");
    }
    this->ss_family = AF_INET;
}

void sockaddr_storage::handle_ipv4_with_port() {
    TRACE2(this,
           " Executing upnplib::sockaddr_storage::handle_ipv4_with_port()")
    this->handle_ipv4();
    this->handle_port();
}
#endif


// Wrapper for a sockaddr structure
// --------------------------------
SockAddr::SockAddr() {
    this->addr_ss.ss_family = AF_INET;
    this->addr = (struct sockaddr*)&this->addr_ss;
}

void SockAddr::addr_set(const std::string& a_text_addr, unsigned short a_port) {
    int ret = inet_pton(this->addr_ss.ss_family, a_text_addr.c_str(),
                        &this->addr_in->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            a_text_addr + "'");
    }
    this->addr_in->sin_port = htons(a_port);
}

std::string SockAddr::addr_get() {
    // Return the text address which is stored in this structure.

    char buf_ntop[INET6_ADDRSTRLEN]{};

    const char* text_addr =
        inet_ntop(this->addr_ss.ss_family, &this->addr_in->sin_addr, buf_ntop,
                  sizeof(buf_ntop));
    if (text_addr == nullptr)
        throw std::invalid_argument(
            "UPnPlib ERR. at */" +                                //
            std::filesystem::path(__FILE__).filename().string() + //
            "[" + std::to_string(__LINE__) + "], " + __FUNCTION__ +
            "(), errid=" + std::to_string(errno) + ": " +
            umock::string_h.strerror(errno));

    return std::string(buf_ntop);
}

unsigned short SockAddr::addr_get_port() {
    return ntohs(this->addr_in->sin_port);
}

std::string SocketAddr::addr_get() { return SockAddr::addr_get(); }

std::string SocketAddr::addr_get(SOCKET a_sockfd) {
    int rc = umock::sys_socket_h.getsockname(
        a_sockfd, (struct sockaddr*)this->addr, &this->addr_len);

    if (rc == -1)
        throw std::runtime_error(
            "UPnPlib ERR. at */" +
            std::filesystem::path(__FILE__).filename().string() + "[" +
            std::to_string(__LINE__) + "], " + __FUNCTION__ + "(" +
            std::to_string(a_sockfd) + "), errid=" + std::to_string(errno) +
            ": systemcall getsockname(" + std::to_string(a_sockfd) + "), " +
            umock::string_h.strerror(errno));

    return SockAddr::addr_get();
}

} // namespace upnplib
