// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-24
//
// If you need more information on gtests how this class works you can temporary
// uncomment #define DEBUG_URL and run the tests with
// ./build/gtests/test_url_class --gtest_filter=UrlClassTestSuite.*
#define DEBUG_URL

#include "upnplib/url.hpp"
#include <iostream>
//#include <fstream>

//
namespace upnplib {

static bool url_is_special(std::string_view a_s) {
    return a_s == "ftp" || a_s == "file" || a_s == "http" || a_s == "https" ||
           a_s == "ws" || a_s == "wss";
}

//
#if false
Url::Url() {
    // Proof to redirect clog to /dev/null, <fstream> is needed
    // save clog stream buffer
    std::streambuf* clog_old = std::clog.rdbuf();
    // Redirect clog
    std::ofstream clog_new("/dev/null");
    std::clog.rdbuf(clog_new.rdbuf());
}

Url::~Url() {
    // restore clog stream buffer
    std::clog.rdbuf(clog_old);
}
#endif

//
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
    m_ser_base_url = "";
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

//
void Url::operator=(const std::string& a_given_url) {

    m_given_url = a_given_url;
    this->clear_private();

    // To understand the parser below please refer to the "URL Living Standard"
    // as noted at the top. I use the same terms so you should be able to see
    // the relations better that way.

    // Remove control character and space and copy to input. Because we copy
    // char by char I use a predefined length on input to avoid additional
    // memory allocation for characters.
    m_input.reserve(m_given_url.size());
    this->clean_and_copy_url_to_input();

    m_state = STATE_SCHEME_START;
    m_buffer.reserve(m_input.size());
    m_pointer = m_input.begin();

    // On the URL standard there is a State Machine used. It parses the inpupt
    // string with a pointer to the string so it should finish at the end of
    // it. The loop of the State Machine finishes regular if state is set to
    // STATE_NO_STATE within the Machine. We guard it to always finish
    // independent from the Machines logic to be on the safe side. Because the
    // m_pointer is decreased sometimes in the State Machine we just add 10 to
    // guard.
    int guard = m_input.size() + 10;

    // Because there are no external events we can use this
    // simple Finite State Machine (fsm):
    for (; guard > 0; m_pointer++, guard--) {
        if (m_state == STATE_NO_STATE)
            break;

        switch (m_state) {
        case STATE_SCHEME_START:
            this->fsm_scheme_start();
            break;
        case STATE_SCHEME:
            this->fsm_scheme();
            break;
        case STATE_NO_SCHEME:
            this->fsm_no_scheme();
            break;
        case STATE_PATH_OR_AUTHORITY:
            this->fsm_path_or_authority();
            break;
        case STATE_SPECIAL_AUTHORITY_SLASHES:
            this->fsm_special_authority_slashes();
            break;
        case STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES:
            this->fsm_special_authority_ignore_slashes();
            break;
        case STATE_AUTHORITY:
            this->fsm_authority();
            break;
        case STATE_FILE:
            this->fsm_file();
            break;
        case STATE_SPECIAL_RELATIVE_OR_AUTHORITY:
            this->fsm_special_relative_or_authority();
            break;
        case STATE_PATH:
            this->fsm_path();
            break;
        case STATE_OPAQUE_PATH:
            this->fsm_opaque_path();
            break;
        default:
            guard = 0;
            break;
        }
    }

    if (guard <= 0) {
        throw std::out_of_range(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", Parsing URL " + __func__ +
                        ". State Machine doesn't finish regular."));
    }
}

//
void Url::clean_and_copy_url_to_input() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'clean_and_copy_url_to_input'.\n";
#endif

    // Copy given URL to input lowercase and remove all control chars and space.
    for (auto it = m_given_url.begin(); it < m_given_url.end(); it++) {
        // control chars are \x00 to \x1F, space = \x20, DEL (backspace) = \x7F
        if (*it > ' ' && *it != '\x7F')
            m_input.push_back(std::tolower(*it));
    }
    if (m_input.size() != m_given_url.size())
        std::clog << "Warning: Removed " << m_given_url.size() - m_input.size()
                  << " ASCII control character or spaces. Using \"" << m_input
                  << "\" now." << std::endl;
}

//
void Url::fsm_scheme_start() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme_start_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    // Check if first character is an lower ASCII alpha.
    // We should have already converted all chars to lower.
    if (std::islower((unsigned char)*m_pointer)) { // needs type cast here

        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back(std::tolower(*m_pointer));

        m_state = STATE_SCHEME;

    } else {

        m_state = STATE_NO_SCHEME;
        m_pointer--;
    }
}

//
void Url::fsm_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    // Check if character is an ASCII lower alphanumeric or U+002B (+), U+002D
    // (-), or U+002E (.).
    if (islower((unsigned char)*m_pointer) || // type cast is needed here
        isdigit((unsigned char)*m_pointer) || *m_pointer == '+' ||
        *m_pointer == '-' || *m_pointer == '.') //
    {
        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back(*m_pointer);

    } else if (*m_pointer == ':') {

        m_scheme = m_buffer;
        m_buffer = "";

        if (m_scheme == "file") {
            if (m_pointer + 2 >= m_input.end() || *(m_pointer + 1) != '/' ||
                *(m_pointer + 2) != '/')
                std::clog << "Warning: 'file' scheme misses \"//\"."
                          << std::endl;
            m_state = STATE_FILE;

        } else if (url_is_special(m_scheme) && m_ser_base_url != "") {
            m_state = STATE_SPECIAL_RELATIVE_OR_AUTHORITY;

        } else if (url_is_special(m_scheme)) {
            m_state = STATE_SPECIAL_AUTHORITY_SLASHES;

        } else if (m_pointer + 1 < m_input.end() && *(m_pointer + 1) == '/') {
            m_state = STATE_PATH_OR_AUTHORITY;
            m_pointer++;

        } else {
            m_path = "";
            m_state = STATE_OPAQUE_PATH;
        }

    } else {

        m_buffer = "";
        m_state = STATE_NO_SCHEME;
    }
}

//
void Url::fsm_no_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'no_scheme_state' with input \"" << m_input
              << "\"\n";
#endif
    std::clog << "Error: no valid scheme found." << std::endl;
    throw std::invalid_argument("Invalid URL: '" + m_input + "'");

    m_state = STATE_NO_STATE;
}

//
void Url::fsm_special_relative_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_relative_or_authority_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    m_state = STATE_NO_STATE;
}

//
void Url::fsm_path_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_or_authority_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer == '/') {
        m_state = STATE_AUTHORITY;
    } else {
        m_state = STATE_PATH;
        m_pointer--;
    }
}

//
void Url::fsm_special_authority_slashes() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_authority_slashes_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    if (m_pointer + 1 < m_input.end() && *m_pointer == '/' &&
        *(m_pointer + 1) == '/') {
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer++;
    } else {
        std::clog << "Warning: ignore slashes on authority." << std::endl;
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer--;
    }
}

//
void Url::fsm_special_authority_ignore_slashes() {
#ifdef DEBUG_URL
    std::clog
        << "DEBUG: Being on 'special_authority_ignore_slashes_state' with \""
        << std::string(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer != '/' && *m_pointer != '\\') {
        m_state = STATE_AUTHORITY;
        m_pointer--;
    } else {
        std::clog << "Warning: '/' or '\\' not expected on authority."
                  << std::endl;
    }
}

//
void Url::fsm_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'authority_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_file() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'file_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

//
void Url::fsm_opaque_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'opaque_path_state' with \""
              << std::string(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

} // namespace upnplib
