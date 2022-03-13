// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-12

#include "upnplib/url.hpp"
#include <algorithm>
#include <stdexcept>

namespace upnplib {

// homer::Url v0.3.0
// =================
// MIT License
// https://github.com/homer6/url

std::string CUrl::getScheme() const { return this->scheme; }

std::string CUrl::getUsername() const { return this->username; }

std::string CUrl::getPassword() const { return this->password; }

std::string CUrl::getHost() const { return this->host; }

unsigned short CUrl::getPort() const {

    if (this->port.size() > 0) {
        return std::atoi(this->port.c_str());
    }

    if (this->scheme == "https")
        return 443;
    if (this->scheme == "http")
        return 80;
    if (this->scheme == "ssh")
        return 22;
    if (this->scheme == "ftp")
        return 21;
    if (this->scheme == "mysql")
        return 3306;
    if (this->scheme == "mongo")
        return 27017;
    if (this->scheme == "mongo+srv")
        return 27017;
    if (this->scheme == "kafka")
        return 9092;
    if (this->scheme == "postgres")
        return 5432;
    if (this->scheme == "postgresql")
        return 5432;
    if (this->scheme == "redis")
        return 6379;
    if (this->scheme == "zookeeper")
        return 2181;
    if (this->scheme == "ldap")
        return 389;
    if (this->scheme == "ldaps")
        return 636;

    return 0;
}

std::string CUrl::getPath() const {

    std::string tmp_path;
    unescape_path(this->path, tmp_path);
    return tmp_path;
}

std::string CUrl::getQuery() const { return this->query; }

std::string CUrl::getFragment() const { return this->fragment; }

std::string CUrl::getFullPath() const {

    std::string full_path;

    if (this->getPath().size()) {
        full_path += this->getPath();
    }

    if (this->getQuery().size()) {
        full_path += "?" + this->getQuery();
    }

    if (this->getFragment().size()) {
        full_path += "#" + this->getFragment();
    }

    return full_path;
}

bool CUrl::isIpv6() const { return this->ipv6_host; }

void CUrl::setSecure(bool secure_) { this->secure = secure_; }

bool CUrl::isSecure() const { return this->secure; }

std::string_view CUrl::captureUpTo(const std::string_view right_delimiter,
                                   const std::string& error_message) {

    this->right_position =
        this->parse_target.find_first_of(right_delimiter, this->left_position);

    if (right_position == std::string::npos && error_message.size()) {
        throw std::runtime_error(error_message);
    }

    std::string_view captured = this->parse_target.substr(
        this->left_position, this->right_position - this->left_position);

    return captured;
}

bool CUrl::moveBefore(const std::string_view right_delimiter) {

    size_t position =
        this->parse_target.find_first_of(right_delimiter, this->left_position);

    if (position != std::string::npos) {
        this->left_position = position;
        return true;
    }

    return false;
}

bool CUrl::existsForward(const std::string_view right_delimiter) {

    size_t position =
        this->parse_target.find_first_of(right_delimiter, this->left_position);

    if (position != std::string::npos) {
        return true;
    }

    return false;
}

void CUrl::set(const std::string& source_string) {

    this->whole_url_storage = source_string; // copy
    this->clearPrivate();

    if (this->whole_url_storage == "") {
        // This is a valid entry.
        return;
    }

    // reset target
    this->parse_target = this->whole_url_storage;

    // scheme
    this->scheme = this->captureUpTo(":", "Expected : in CUrl");
    std::transform(
        this->scheme.begin(), this->scheme.end(), this->scheme.begin(),
        [](std::string_view::value_type c) { return std::tolower(c); });
    this->left_position += scheme.size() + 1;

    // authority

    if (this->moveBefore("//")) {
        this->authority_present = true;
        this->left_position += 2;
    }

    if (this->authority_present) {

        this->authority = this->captureUpTo("/");

        bool path_exists = false;

        if (this->moveBefore("/")) {
            path_exists = true;
        }

        if (this->existsForward("?")) {

            this->path = this->captureUpTo("?");
            this->moveBefore("?");
            this->left_position++;

            if (this->existsForward("#")) {
                this->query = this->captureUpTo("#");
                this->moveBefore("#");
                this->left_position++;
                this->fragment = this->captureUpTo("#");
            } else {
                // no fragment
                this->query = this->captureUpTo("#");
            }

        } else {

            // no query
            if (this->existsForward("#")) {
                this->path = this->captureUpTo("#");
                this->moveBefore("#");
                this->left_position++;
                this->fragment = this->captureUpTo("#");
            } else {
                // no fragment
                if (path_exists) {
                    this->path = this->captureUpTo("#");
                }
            }
        }

    } else {

        this->path = this->captureUpTo("#");
    }

    // parse authority

    // reset target
    this->parse_target = this->authority;
    this->left_position = 0;
    this->right_position = 0;

    if (this->existsForward("@")) {

        this->user_info = this->captureUpTo("@");
        this->moveBefore("@");
        this->left_position++;

    } else {
        // no user_info
    }

    // detect ipv6
    if (this->existsForward("[")) {
        this->left_position++;
        this->host = this->captureUpTo("]", "Malformed ipv6");
        this->left_position++;
        this->ipv6_host = true;
    } else {

        if (this->existsForward(":")) {
            this->host = this->captureUpTo(":");
            this->moveBefore(":");
            this->left_position++;
            this->port = this->captureUpTo("#");
        } else {
            // no port
            this->host = this->captureUpTo(":");
        }
    }

    // parse user_info

    // reset target
    this->parse_target = this->user_info;
    this->left_position = 0;
    this->right_position = 0;

    if (this->existsForward(":")) {

        this->username = this->captureUpTo(":");
        this->moveBefore(":");
        this->left_position++;

        this->password = this->captureUpTo("#");

    } else {
        // no password

        this->username = this->captureUpTo(":");
    }

    // update secure
    if (this->scheme == "ssh" || this->scheme == "https" ||
        this->port == "443") {
        this->secure = true;
    }

    if (this->scheme == "postgres" || this->scheme == "postgresql") {

        // reset parse target to query
        this->parse_target = this->query;
        this->left_position = 0;
        this->right_position = 0;

        if (this->existsForward("ssl=true")) {
            this->secure = true;
        }
    }
}

void CUrl::clear() {

    this->whole_url_storage = "";
    return this->clearPrivate();
}

void CUrl::clearPrivate() {

    // clear all properties to valid empty values except whole_url_storage that
    // should be initialized as very first statement.
    this->scheme = "";
    this->authority = "";
    this->user_info = "";
    this->username = "";
    this->password = "";
    this->host = "";
    this->port = "";
    this->path = "";
    this->query = "";
    this->fragment = "";

    this->secure = false;
    this->ipv6_host = false;
    this->authority_present = false;

    this->left_position = 0;
    this->right_position = 0;
    this->parse_target = "";

    return;
}

bool CUrl::unescape_path(const std::string& in, std::string& out) {

    out.clear();
    out.reserve(in.size());

    for (std::size_t i = 0; i < in.size(); ++i) {

        switch (in[i]) {

        case '%':

            if (i + 3 <= in.size()) {

                unsigned int value = 0;

                for (std::size_t j = i + 1; j < i + 3; ++j) {

                    switch (in[j]) {

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        value += in[j] - '0';
                        break;

                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                        value += in[j] - 'a' + 10;
                        break;

                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'E':
                    case 'F':
                        value += in[j] - 'A' + 10;
                        break;

                    default:
                        return false;
                    }

                    if (j == i + 1)
                        value <<= 4;
                }

                out += static_cast<char>(value);
                i += 2;

            } else {

                return false;
            }

            break;

        case '-':
        case '_':
        case '.':
        case '!':
        case '~':
        case '*':
        case '\'':
        case '(':
        case ')':
        case ':':
        case '@':
        case '&':
        case '=':
        case '+':
        case '$':
        case ',':
        case '/':
        case ';':
            out += in[i];
            break;

        default:
            if (!std::isalnum(in[i]))
                return false;
            out += in[i];
            break;
        }
    }

    return true;
}

bool operator==(const CUrl& a, const CUrl& b) {

    return a.scheme == b.scheme && a.username == b.username &&
           a.password == b.password && a.host == b.host && a.port == b.port &&
           a.path == b.path && a.query == b.query && a.fragment == b.fragment;
}

bool operator!=(const CUrl& a, const CUrl& b) { return !(a == b); }

bool operator<(const CUrl& a, const CUrl& b) {

    if (a.scheme < b.scheme)
        return true;
    if (b.scheme < a.scheme)
        return false;

    if (a.username < b.username)
        return true;
    if (b.username < a.username)
        return false;

    if (a.password < b.password)
        return true;
    if (b.password < a.password)
        return false;

    if (a.host < b.host)
        return true;
    if (b.host < a.host)
        return false;

    if (a.port < b.port)
        return true;
    if (b.port < a.port)
        return false;

    if (a.path < b.path)
        return true;
    if (b.path < a.path)
        return false;

    if (a.query < b.query)
        return true;
    if (b.query < a.query)
        return false;

    return a.fragment < b.fragment;
}

std::string CUrl::toString() const { return this->whole_url_storage; }

// CUrl::operator std::string() const { return this->toString(); }

} // namespace upnplib
