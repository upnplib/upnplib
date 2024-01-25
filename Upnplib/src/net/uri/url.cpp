// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-25
/*!
 * \file
 * \brief Definition of the 'class Url'. Not usable, work in progess.
 */

// TODO: Provide url_is_special() as flag

#include "upnplib/url.hpp"

#include <map>
#include <sstream>
#include <iomanip>
#include <iostream>
/// \cond

namespace {

const std::map<const std::string, const uint16_t> special_scheme{
    {"file", static_cast<uint16_t>(NULL)},
    {"ftp", static_cast<uint16_t>(21)},
    {"http", static_cast<uint16_t>(80)},
    {"https", static_cast<uint16_t>(443)},
    {"ws", static_cast<uint16_t>(80)},
    {"wss", static_cast<uint16_t>(443)}};

bool url_is_special(std::string_view a_str) {
    return a_str == "ftp" || a_str == "file" || a_str == "http" ||
           a_str == "https" || a_str == "ws" || a_str == "wss";
}

bool is_in_userinfo_percent_encode_set(const unsigned char a_chr) {
    return // C0 controls
        a_chr <= '\x1F' ||
        // C0 control percent-encode set
        a_chr > '\x7E' ||
        // query percent-encode set
        a_chr == ' ' || a_chr == '"' || a_chr == '#' || a_chr == '<' ||
        a_chr == '>' ||
        // path percent-encode set
        a_chr == '?' || a_chr == '`' || a_chr == '{' || a_chr == '}' ||
        // userinfo percent-encode set
        a_chr == '/' || a_chr == ':' || a_chr == ';' || a_chr == '=' ||
        a_chr == '@' || (a_chr >= '[' && a_chr <= '^') || a_chr == '|';
}

std::string UTF8_percent_encode(const unsigned char a_chr) {
    // Simplified function 'UTF-8 percent-encode' from the URL standard may be
    // adjusted if needed.
    if (is_in_userinfo_percent_encode_set(a_chr)) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::uppercase << std::hex;
        escaped << '%' << std::setw(2) << int(a_chr);
        return escaped.str();
    } else
        return std::string(sizeof(a_chr), (char)a_chr);
}

std::string esc_url(std::string_view a_str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::uppercase << std::hex;

    for (const char chr : a_str) {
        if ((unsigned char)chr <= '\x1F' || (unsigned char)chr > '\x7E')
            escaped << '%' << std::setw(2) << int((unsigned char)chr);
        else
            escaped << (unsigned char)chr;
    }
    return escaped.str();
}

} // namespace

namespace upnplib {


// Url class methods
// =================

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


Url::operator std::string() const { return m_ser_url; }

void Url::clear() {
    m_given_url = "";
    this->clear_private();
}

void Url::clear_private() {
    // Clears all properties except m_given_url that may already be set to a new
    // value.
    m_input.reserve(m_given_url.size());
    m_input = "";
    m_buffer.reserve(m_input.size() + 20);
    m_buffer = "";
    m_ser_url = "";
    m_ser_base_url = "";
    m_scheme = "";
    m_authority = "";
    m_username = "";
    m_password = "";
    m_host = "";
    m_port = "";
    m_port_num = (uint16_t)NULL;
    m_path = "";
    m_query = "";
    m_fragment = "";
    m_atSignSeen = false;
    m_insideBrackets = false;
    m_passwordTokenSeen = false;
}

std::string Url::scheme() const { return m_scheme; }

std::string Url::authority() const { return m_authority; }

std::string Url::username() const { return m_username; }

std::string Url::password() const { return m_password; }

std::string Url::host() const { return m_host; }

std::string Url::port() const { return m_port; }

uint16_t Url::port_num() const { return m_port_num; }

std::string Url::path() const { return m_path; }

std::string Url::query() const { return m_query; }

std::string Url::fragment() const { return m_fragment; }


void Url::operator=(std::string_view a_given_url) {

    m_given_url = a_given_url;
    this->clear_private();

    // To understand the parser below please refer to the "URL Living Standard"
    // as noted at the top. I use the same terms so you should be able to see
    // the relations better that way.

    // Remove control character and space and copy to input. Because we copy
    // char by char I use a predefined length on input to avoid additional
    // memory allocation for characters.
    this->clean_and_copy_url_to_input();

    m_state = STATE_SCHEME_START;
    m_pointer = m_input.begin();

    // On the URL standard there is a State Machine used. It parses the inpupt
    // string with a pointer to the string so it should finish at the end of
    // it. The loop of the State Machine finishes regular if state is set to
    // STATE_NO_STATE within the Machine. We guard it to always finish
    // independent from the Machines logic to be on the safe side. Because the
    // m_pointer is decreased sometimes in the State Machine and maybe several
    // code points are percent encoded (will increase m_input) we double guard.
    // But we need at least two loops to regular finish an empty m_input.
    size_t guard = m_input.size() * 2 + 2;
#ifdef DEBUG_URL
    std::clog << "DEBUG: guard = " << guard << std::endl;
#endif

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
        case STATE_HOST:
            this->fsm_host();
            break;
        case STATE_PORT:
            this->fsm_port();
            break;
        case STATE_FILE:
            this->fsm_file();
            break;
        case STATE_SPECIAL_RELATIVE_OR_AUTHORITY:
            this->fsm_special_relative_or_authority();
            break;
        case STATE_PATH_START:
            this->fsm_path_start();
            break;
        case STATE_PATH:
            this->fsm_path();
            break;
        case STATE_OPAQUE_PATH:
            this->fsm_opaque_path();
            break;
        default:
            // Undefined state, stop the State Machine irregular. This is a
            // program bug.
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


void Url::clean_and_copy_url_to_input() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'clean_and_copy_url_to_input'.\n";
#endif

    // To remove any leading C0 control or space, point to first valid char.
    // control chars are \x00 to \x1F, space = \x20, DEL (backspace) = \x7F.
    // Due to the URL standard backspace is ignored here.
    auto it_leading = m_given_url.begin();
    auto str_end = m_given_url.end();
    while (it_leading < str_end && (unsigned char)*it_leading <= ' ') {
        if (it_leading >= str_end - 1)
            break;
        it_leading++;
    }
    // std::clog << "  DEBUG: *it_leading = '" << *it_leading << "'\n";

    // To remove any trailing C0 control or space, point to last valid char.
    auto it_trailing = m_given_url.end() - 1;
    auto str_begin = m_given_url.begin();
    while (it_trailing >= str_begin && (unsigned char)*it_trailing <= ' ') {
        if (it_trailing <= str_begin)
            break;
        else
            it_trailing--;
    }
    // std::clog << "  DEBUG: *it_trailing = '" << *it_trailing << "'\n";

    // Copy given URL to input lowercase and remove all ASCII tab or newline.
    int invalid_chars{};
    if ((unsigned char)*it_leading > ' ') {

        // it_leading points to the first valid character,
        // it_trailing points to the last valid character.
        while (it_leading <= it_trailing) {

            unsigned char c = (unsigned char)*it_leading;
            if (c == '\x0D' || c == '\x0A' || c == '\x09')
                invalid_chars++;
            else
                m_input.push_back((char)std::tolower(c));
            it_leading++;
        }
    }
    // std::clog << "  DEBUG: m_input = '" << m_input << "'\n";

    if (invalid_chars)
        std::clog << "Warning: Removed " << invalid_chars
                  << " ASCII tab or newline character. Using \"" << m_input
                  << "\" now." << std::endl;
}


void Url::fsm_scheme_start() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme_start_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    // Check if first character is an lower ASCII alpha.
    // We should have already converted all chars to lower.
    if (std::islower((unsigned char)*m_pointer)) { // needs type cast here

        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back((char)std::tolower(*m_pointer));

        m_state = STATE_SCHEME;

    } else {

        m_state = STATE_NO_SCHEME;
        m_pointer--;
    }
}


void Url::fsm_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'scheme state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    const unsigned char c =
        m_pointer < m_input.end() ? (unsigned char)*m_pointer : '\0';

    // Check if character is an ASCII lower alphanumeric or U+002B (+), U+002D
    // (-), or U+002E (.).
    if (islower(c) || // type cast is needed here
        isdigit(c) || c == '+' || c == '-' || c == '.') //
    {
        // Exception: if the operation would result in size() > max_size(),
        // throws std::length_error.
        m_buffer.push_back((char)c);

    } else if (c == ':') {

        m_scheme = m_buffer;
        m_buffer = "";

        if (m_scheme == "file") {
            if (m_pointer + 2 >= m_input.end() || *(m_pointer + 1) != '/' ||
                *(m_pointer + 2) != '/')
                std::clog << "Warning: 'file' scheme misses \"//\", ignoring."
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


void Url::fsm_no_scheme() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'no_scheme_state' with input \"" << m_input
              << "\"\n";
#endif
    std::clog << "Error: no valid scheme found." << std::endl;
    throw std::invalid_argument("Invalid URL: '" + esc_url(m_input) + "'");

    m_state = STATE_NO_STATE;
}


void Url::fsm_special_relative_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_relative_or_authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    m_state = STATE_NO_STATE;
}


void Url::fsm_path_or_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_or_authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer == '/') {
        m_state = STATE_AUTHORITY;
    } else {
        m_state = STATE_PATH;
        m_pointer--;
    }
}


void Url::fsm_special_authority_slashes() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'special_authority_slashes_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (m_pointer + 1 < m_input.end() && *m_pointer == '/' &&
        *(m_pointer + 1) == '/') {
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer++;
    } else {
        std::clog << "Warning: no \"//\" before authority: ignoring. Found \""
                  << std::string(m_pointer, m_input.end()) << "\"" << std::endl;
        m_state = STATE_SPECIAL_AUTHORITY_IGNORE_SLASHES;
        m_pointer--;
    }
}


void Url::fsm_special_authority_ignore_slashes() {
#ifdef DEBUG_URL
    std::clog
        << "DEBUG: Being on 'special_authority_ignore_slashes_state' with \""
        << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    if (*m_pointer != '/' && *m_pointer != '\\') {
        m_state = STATE_AUTHORITY;
        m_pointer--;
    } else {
        std::clog << "Warning: '/' or '\\' not expected on authority: "
                     "ignoring. Found \""
                  << std::string(m_pointer, m_input.end()) << "\"" << std::endl;
    }
}


void Url::fsm_authority() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'authority_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif

    const unsigned char c =
        m_pointer < m_input.end() ? (unsigned char)*m_pointer : '\0';

    if (c == '@') {

        std::clog << "Status: '@' found for userinfo." << std::endl;
        if (m_atSignSeen)
            m_buffer.append("%40");
        else
            m_atSignSeen = true;

        for (auto& cp : m_buffer) {
            if (cp == ':' && !m_passwordTokenSeen) {
                m_passwordTokenSeen = true;
                continue;
            }
            std::string encodedCodePoints =
                UTF8_percent_encode((unsigned char)cp);
            if (m_passwordTokenSeen)
                m_password += encodedCodePoints;
            else
                m_username += encodedCodePoints;
        }
        m_buffer = "";

    } else if (m_pointer >= m_input.end() || c == '/' || c == '?' || c == '#' ||
               (url_is_special(m_scheme) && c == '\\')) {

        if (m_atSignSeen && m_buffer == "") {
            std::clog << "Error: no valid authority." << std::endl;
            throw std::invalid_argument("Invalid authority: '" + m_input + "'");
        } else {
            m_pointer = m_pointer - (long int)m_buffer.length() - 1;
            m_buffer = "";
            m_state = STATE_HOST;
        }

    } else {
        m_buffer.push_back((char)c);
    }
}


void Url::fsm_host() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'host_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\", "
              << "username = \"" << m_username << "\", password = \""
              << m_password << "\"\n";
#endif

    const unsigned char c =
        m_pointer < m_input.end() ? (unsigned char)*m_pointer : '\0';

    if (c == ':' && !m_insideBrackets) {

        if (m_buffer.empty()) {
            std::clog << "Error: no valid hostname found." << std::endl;
            throw std::invalid_argument("Invalid hostname: '" +
                                        esc_url(m_input) + "'");
        }
        // On failure host_parser throws an error that is catched call stack
        // upwards. It doesn't modify m_host then.
        // m_host = host_parser(m_buffer, /* isNotSpecial */ true);
        m_host = "dummy1.host.state";
        m_buffer.clear();
        m_state = STATE_PORT;

    } else if (c == '\0' || c == '/' || c == '?' || c == '#' ||
               (url_is_special(m_scheme) && c == '\\')) {

        m_pointer--;

        if (url_is_special(m_scheme) && m_buffer.empty()) {
            std::clog << "Error: no valid host found." << std::endl;
            throw std::invalid_argument("Invalid hostname: '" +
                                        esc_url(m_input) + "'");
        }
        // On failure host_parser throws an error that is catched call stack
        // upwards. It doesn't modify m_host then.
        // m_host = host_parser(m_buffer, /* isNotSpecial */ true);
        m_host = "dummy2.host.state";
        m_buffer.clear();
        m_state = STATE_PATH_START;

    } else {
        if (c == '[')
            m_insideBrackets = true;
        else if (c == ']')
            m_insideBrackets = false;

        m_buffer.push_back((char)c);
    }
}


void Url::fsm_port() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'port_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\", "
              << "host = \"" << m_host << "\"\n";
#endif

    const unsigned char c =
        m_pointer < m_input.end() ? (unsigned char)*m_pointer : '\0';

    if (std::isdigit(c)) {
        m_buffer.push_back((char)c);

    } else if (c == '\0' || c == '/' || c == '?' || c == '#' ||
               (url_is_special(m_scheme) && c == '\\')) {

        if (!m_buffer.empty()) {
            // uint16_t limits port number to max 65535
            uint16_t port{};
            long unsigned int port_tmp{};

            // If port is greater than 2^16 − 1 (65535), validation error,
            // return failure.
            try {
                port_tmp = std::stoul(m_buffer);
                // } catch(std::invalid_argument& e) {} // not catched here
            } catch (std::out_of_range& e) {
                std::clog << "Error: " << e.what()
                          << ". Port number out of range." << std::endl;
                throw;
            }
            if (port_tmp > 65535) {
                throw std::out_of_range(std::string(
                    (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                    ", Parsing port " + __func__ + ". Error: Port number " +
                    std::to_string(port_tmp) + " is out of range."));
            }
            port = (uint16_t)port_tmp; // type cast is checked

            // Set url’s port to null, if port is url’s scheme’s default port;
            // otherwise to port.
            auto it = special_scheme.find(m_scheme);
            if (it != special_scheme.end() && it->second == port) {
                m_port_num = (uint16_t)NULL;
                m_port.clear();
            } else {
                m_port_num = port;
                m_port = m_buffer;
            }
            m_buffer.clear();
        }

        m_state = STATE_PATH_START;
        m_pointer--;

    } else {
        std::clog << "Error: no valid port found." << std::endl;
        throw std::invalid_argument("Invalid port: '" + esc_url(m_input) + "'");
    }
}


void Url::fsm_file() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'file_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}


void Url::fsm_path_start() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_start_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}


void Url::fsm_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'path_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}


void Url::fsm_opaque_path() {
#ifdef DEBUG_URL
    std::clog << "DEBUG: Being on 'opaque_path_state' with \""
              << std::string_view(m_pointer, m_input.end()) << "\"\n";
#endif
    m_state = STATE_NO_STATE;
}

} // namespace upnplib
/// \endcond
