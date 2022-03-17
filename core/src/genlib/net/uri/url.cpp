// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-17

#include "upnplib/url.hpp"
#include <memory>
#include <cstring>
#include <iostream>

namespace upnplib {

Url::operator std::string() const { return m_serialized_url; }

void Url::clear() {
    m_given_url = "";
    this->clear_private();
}

void Url::clear_private() {
    // Clears all properties except m_given_url that may already be set to a new
    // value.
    m_input = "";
    m_buffer = "";
    m_serialized_url = "";
    m_scheme = "";
    m_authority = "";
    m_userinfo = "";
    m_host = "";
    m_port = "";
    m_port_num = 0;
    m_path = "";
    m_query = "";
    m_fragment = "";
}

std::string Url::scheme() const { return m_scheme; }

std::string Url::authority() const { return m_authority; }

std::string Url::userinfo() const { return m_userinfo; }

std::string Url::host() const { return m_host; }

std::string Url::port() const { return m_port; }

uint16_t Url::port_num() const { return m_port_num; }

std::string Url::path() const { return m_path; }

std::string Url::query() const { return m_query; }

std::string Url::fragment() const { return m_fragment; }

void Url::operator=(const std::string& a_given_url) {

    m_given_url = a_given_url;
    this->clear_private();

    // std::cout << "DEBUG: m_given_url.size() = " << m_given_url.size() <<
    // '\n'; std::cout << "DEBUG: m_given_url = '" << m_given_url << "'\n";

    if (m_given_url == "")
        return;

    enum { STATE_NO_STATE, STATE_SCHEME_START, STATE_SCHEME, STATE_NO_SCHEME };
    int current_state{STATE_NO_STATE};

    // Following this [basic URL parser]
    // (https://url.spec.whatwg.org/#concept-basic-url-parser)
    // without optional parameter means we parse only the string input.
    // To understand the parser below please refer this standard.
    // I use the same variable names with additional prefix 'm_'.

    // Remove any leading and trailing C0 control or space from input.
    m_input = m_given_url;
    this->fsm_cleanup_input();

    current_state = STATE_SCHEME_START;
    m_buffer = "";
    m_pointer = m_input.begin();

    // Because there are no external events we can use this
    // simple state machine:
    std::cout << "DEBUG: *m_pointer = ";
    for (; m_pointer < m_input.end(); m_pointer++) {
        switch (current_state) {
        case STATE_SCHEME_START:
            this->fsm_scheme_start();
            break;
        case STATE_SCHEME:
            // do something in the stop state
            break;
        case STATE_NO_SCHEME:
            // do something in the stop state
            break;
        }
    }
    std::cout << std::endl;
}

//
void Url::fsm_cleanup_input() {

    // Remove leading control or space
    auto ptr = m_input.begin();
    for (; ptr < m_input.end(); ptr++) {
        if (*ptr > ' ')
            break;
    }
    if (ptr > m_input.begin()) {
        m_input.erase(m_input.begin(), ptr);
        std::clog << "Warning: Leading control or space characters removed "
                     "from URL. Using '"
                  << m_input << "' now." << std::endl;
    }

    // std::cout << "DEBUG: m_input.size() = " << m_input.size() << '\n';
    // std::cout << "DEBUG: m_input = '" << m_input << "'\n";

    // Remove trailing control or space
    ptr = m_input.end() - 1;
    for (; ptr >= m_input.begin(); ptr--) {
        if (*ptr > ' ')
            break;
    }
    if (ptr < m_input.end() - 1) {
        m_input.erase(ptr + 1, m_input.end());
        std::clog << "Warning: Trailing control or space characters removed "
                     "from URL. Using '"
                  << m_input << "' now." << std::endl;
    }

    // std::cout << "DEBUG: m_input.size() = " << m_input.size() << '\n';
    // std::cout << "DEBUG: m_input = '" << m_input << "'\n";

    // Remove all ASCII tab or newline from input.
    auto old_size = m_input.size();
    m_input.erase(std::remove_if(m_input.begin(), m_input.end(),
                                 [](char c) {
                                     return c == '\t' || c == '\n' || c == '\r';
                                 }),
                  m_input.end());
    if (m_input.size() != old_size)
        std::clog << "Warning: Removed " << old_size - m_input.size()
                  << " ASCII tab or newline character from URL. Using '"
                  << m_input << "' now." << std::endl;

    return;
}

//
void Url::fsm_scheme_start() { std::cout << *m_pointer; }

} // namespace upnplib
