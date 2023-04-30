// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-30

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


// Free function to get the address string from a sockaddr structure
// -----------------------------------------------------------------
std::string to_addr_str(const ::sockaddr_storage* const a_sockaddr) {
    TRACE("Executing upnplib::to_addr_str()")
    char addrbuf[INET6_ADDRSTRLEN]{};

    switch (a_sockaddr->ss_family) {
    case AF_UNSPEC:
        return "";

    case AF_INET6:
        ::inet_ntop(AF_INET6, &((sockaddr_in6*)a_sockaddr)->sin6_addr.s6_addr,
                    addrbuf, sizeof(addrbuf));
        return '[' + std::string(addrbuf) + ']';

    case AF_INET:
        ::inet_ntop(AF_INET, &((sockaddr_in*)a_sockaddr)->sin_addr.s_addr,
                    addrbuf, sizeof(addrbuf));
        return std::string(addrbuf);

    default:
        throw std::invalid_argument("ERROR! Unsupported address family " +
                                    std::to_string(a_sockaddr->ss_family));
    }
}


// Specialized sockaddr_structure
// ------------------------------
// Constructor
SSockaddr_storage::SSockaddr_storage() {
    TRACE2(this, " Construct upnplib::SSockaddr_storage()") //
}

// Destructor
SSockaddr_storage::~SSockaddr_storage() {
    TRACE2(this, " Destruct upnplib::SSockaddr_storage()") //
}

// Assignment operator to set socket address from string,
void SSockaddr_storage::operator=(const std::string& a_addr_str) {
    // Valid input examles: "[2001:db8::1]", "[2001:db8::1]:50001",
    //                      "192.168.1.1", "192.168.1.1:50001".
    TRACE2(this, " Executing upnplib::SSockaddr_storage::operator=()")

    // An empty address string clears the address storage
    if (a_addr_str.empty()) {
        memset(&this->ss, 0, sizeof(this->ss));
        return;
    }

    if (a_addr_str.front() == '[') {    // IPv6 address
        if (a_addr_str.back() == ']') { // IPv6 address without port
            this->handle_ipv6(a_addr_str);

        } else { // IPv6 with port
            // Split address and port
            size_t pos = a_addr_str.find("]:");
            this->handle_ipv6(a_addr_str.substr(0, pos + 1));
            this->handle_port(a_addr_str.substr(pos + 2));
        }
    } else { // IPv4 address or port
        size_t pos = a_addr_str.find_first_of(":");
        if (pos != std::string::npos) { // ':' found means ipv4 with port
            this->handle_ipv4(a_addr_str.substr(0, pos));
            this->handle_port(a_addr_str.substr(pos + 1));

        } else { // IPv4 without port or port only
            if (a_addr_str.find_first_of(".") != std::string::npos) {
                this->handle_ipv4(a_addr_str); // IPv4 address without port

            } else { // port only
                this->handle_port(a_addr_str);
            }
        }
    }
}

void SSockaddr_storage::handle_ipv6(const std::string& a_addr_str) {
    TRACE2(this, " Executing upnplib::SSockaddr_storage::handle_ipv6()")
    // remove surounding brackets
    std::string addr_str = a_addr_str.substr(1, a_addr_str.length() - 2);

    int ret = inet_pton(AF_INET6, addr_str.c_str(),
                        &((sockaddr_in6*)&this->ss)->sin6_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            a_addr_str + "'");
    }
    this->ss.ss_family = AF_INET6;
}

void SSockaddr_storage::handle_ipv4(const std::string& a_addr_str) {
    TRACE2(this, " Executing upnplib::SSockaddr_storage::handle_ipv4()")
    int ret = inet_pton(AF_INET, a_addr_str.c_str(),
                        &((sockaddr_in*)&this->ss)->sin_addr);
    if (ret == 0) {
        throw std::invalid_argument(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Invalid ip address '" +
            a_addr_str + "'");
    }
    this->ss.ss_family = AF_INET;
}

void SSockaddr_storage::handle_port(const std::string& a_port) {
    TRACE2(this, " Executing upnplib::SSockaddr_storage::handle_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    ((sockaddr_in6*)&this->ss)->sin6_port = htons(to_port(a_port));
}


std::string SSockaddr_storage::get_addr_str() const {
    TRACE2(this, " Executing upnplib::SSockaddr_storage::get_addr_str()")
    return to_addr_str(&this->ss);
}

uint16_t SSockaddr_storage::get_port() const {
    TRACE2(this, " Executing upnplib::SSockaddr_storage::get_port()")
    // sin_port and sin6_port are on the same memory location (union of the
    // structures) so we can use it for AF_INET and AF_INET6.
    return ntohs(((sockaddr_in6*)&this->ss)->sin6_port);
}


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
