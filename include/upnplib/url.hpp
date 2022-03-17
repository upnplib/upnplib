// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-17

// As far as possible is this class based on
// [URL - Living Standard](https://url.spec.whatwg.org/).

#ifndef UPNPLIB_NET_URI_URL_HPP
#define UPNPLIB_NET_URI_URL_HPP

#include <string>

namespace upnplib {

class Url {

  public:
    // Set url, e.g.: Url url; url = "http://example.com"
    void operator=(const std::string& a_url);

    // Get serialized url, e.g.: ser_url = (std::string)url
    operator std::string() const;

    void clear();

    // getter
    std::string scheme() const;
    std::string authority() const;
    std::string userinfo() const;
    std::string host() const;
    std::string port() const;
    uint16_t port_num() const;
    std::string path() const;
    std::string query() const;
    std::string fragment() const;

  private:
    // The strings are initialized to "" by its constructor.
    std::string m_given_url; // unmodified input to the object
    std::string m_input;     // cleaned up copy of m_given_url
    std::basic_string<char>::iterator
        m_pointer; // will hold a pointer to m_input
    std::string m_buffer;
    std::string m_serialized_url;
    std::string m_scheme;
    std::string m_authority;
    std::string m_userinfo;
    std::string m_host;
    std::string m_port;
    uint16_t m_port_num{0};
    std::string m_path;
    std::string m_query;
    std::string m_fragment;

    void clear_private();

    // Methods for the simple Finite State Machine
    void fsm_cleanup_input();
    void fsm_scheme_start();
};

} // namespace upnplib

#endif // UPNPLIB_NET_URI_URL_HPP
