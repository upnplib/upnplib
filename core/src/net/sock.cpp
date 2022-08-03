// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-26

#include "upnplib/sock.hpp"
#include <filesystem>

namespace upnplib {

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
    const char* text_addr =
        inet_ntop(this->addr_ss.ss_family, &this->addr_in->sin_addr,
                  this->buf_ntop, sizeof(this->buf_ntop));
    if (text_addr == nullptr) {
        throw std::runtime_error(
            "at */" + std::filesystem::path(__FILE__).filename().string() +
            "[" + std::to_string(__LINE__) + "]: Got invalid ip address");
    }
    return std::string(this->buf_ntop);
}

unsigned short SockAddr::addr_get_port() {
    return ntohs(this->addr_in->sin_port);
}

} // namespace upnplib
