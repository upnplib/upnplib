// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-29

// This class is based on the "URL Living Standard"
// ================================================
// At time the Url parser was coded this
// Commit Snapshot — Last Updated 21 February 2022 of the URL standard:
// https://url.spec.whatwg.org/commit-snapshots/9a83e251778046b20f4822f15ad4e2a469de2f57//
// was used.
//
// To understand the parser follow there the [basic URL parser]
// (https://url.spec.whatwg.org/#concept-basic-url-parse://url.spec.whatwg.org/commit-snapshots/9a83e251778046b20f4822f15ad4e2a469de2f57/#concept-basic-url-parser)
//
//  To have it available here:
//  "A URI is opaque if, and only if, it is absolute and its scheme-specific
//  part does not begin with a slash character ('/'). An opaque URI has a
//  scheme, a scheme-specific part, and possibly a fragment; all other
//  components are undefined." E.g.: mailto:a@b
//  (http://docs.oracle.com/javase/8/docs/api/java/net/URI.html#isOpaque--)
//
// To manual verify URLs conforming to the standard you can use the
// [Live URL Viewer](https://jsdom.github.io/whatwg-url/).

#ifndef UPNPLIB_NET_URI_URL_HPP
#define UPNPLIB_NET_URI_URL_HPP

namespace upnplib {

#if false
class CIpv6Parser {
  public:
    CIpv6Parser() : m_hints(new addrinfo), m_result(nullptr, &freeaddrinfo);
    in6_addr get(const std::string& a_input);

  private:
    std::unique_ptr<addrinfo> m_hints;
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> m_result;
};
#endif

//
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
    std::string username() const;
    std::string password() const;
    std::string host() const;
    std::string port() const;
    uint16_t port_num() const;
    std::string path() const;
    std::string query() const;
    std::string fragment() const;

  private:
    // The strings are initialized to "" by its constructor.
    std::string m_given_url; // unmodified input to the object
    std::string m_ser_url;
    std::string m_ser_base_url;
    std::string m_scheme;
    std::string m_authority;
    std::string m_username;
    std::string m_password;
    std::string m_host;
    std::string m_port;        // Always set to the current port
    uint16_t m_port_num{NULL}; // Only set if not default port of the scheme
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
        STATE_HOST,
        STATE_PORT,
        STATE_FILE,
        STATE_PATH_START,
        STATE_PATH,
        STATE_OPAQUE_PATH,
        STATE_SPECIAL_RELATIVE_OR_AUTHORITY,
    };
    int m_state{STATE_NO_STATE};

    std::string m_input; // cleaned up copy of m_given_url for the state machine
    std::string::iterator m_pointer; // will hold a pointer to m_input
    std::string m_buffer;
    bool m_atSignSeen;
    bool m_insideBrackets;
    bool m_passwordTokenSeen;

    void clean_and_copy_url_to_input();
    void fsm_scheme_start();
    void fsm_scheme();
    void fsm_no_scheme();
    void fsm_path_or_authority();
    void fsm_special_authority_slashes();
    void fsm_special_authority_ignore_slashes();
    void fsm_authority();
    void fsm_host();
    void fsm_port();
    void fsm_file();
    void fsm_path_start();
    void fsm_path();
    void fsm_opaque_path();
    void fsm_special_relative_or_authority();
};

} // namespace upnplib

#endif // UPNPLIB_NET_URI_URL_HPP
