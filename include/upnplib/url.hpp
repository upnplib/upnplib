// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-22

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
    std::string m_serialized_url;
    std::string m_ser_base_url;
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

    // Settings for the simple Finite State Machine
    enum {
        STATE_NO_STATE,
        STATE_SCHEME_START,
        STATE_SCHEME,
        STATE_NO_SCHEME,
        STATE_PATH_OR_AUTHORITY,
        STATE_SPECIAL_AUTHORITY_SLASHES,
        STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES,
        STATE_AUTHORITY,
        STATE_FILE,
        STATE_PATH,
        STATE_OPAQUE_PATH,
        STATE_SPECIAL_RELATIVE_OR_AUTHORITY,
    };
    int m_state{STATE_NO_STATE};

    std::string m_input; // cleaned up copy of m_given_url for the state machine
    std::string::const_iterator m_pointer; // will hold a pointer to m_input
    std::string m_buffer;

    void copy_url_clean_to_input();
    void fsm_scheme_start();
    void fsm_scheme();
    void fsm_no_scheme();
    void fsm_path_or_authority();
    void fsm_special_authority_slashes();
    void fsm_special_authority_ignore_slashes();
    void fsm_authority();
    void fsm_file();
    void fsm_path();
    void fsm_opaque_path();
    void fsm_special_relative_or_authority();
};

} // namespace upnplib

#endif // UPNPLIB_NET_URI_URL_HPP
