// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-12

#ifndef UPNPLIB_NET_URI_URL_HPP
#define UPNPLIB_NET_URI_URL_HPP

#include <string>

namespace upnplib {

// homer::Url v0.3.0
// =================
// MIT License
// https://github.com/homer6/url

// This class takes inspiration and some source code from
// https://github.com/chriskohlhoff/urdl/blob/master/include/urdl/url.hpp
/*
    Url and UrlView are compliant with
        https://tools.ietf.org/html/rfc3986
        https://tools.ietf.org/html/rfc6874
        https://tools.ietf.org/html/rfc7320
        and adheres to https://rosettacode.org/wiki/URL_parser examples.
    Url will use default ports for known schemes, if the port is not explicitly
   provided.
*/

class CUrl {

  public:
    void set(const std::string& s);
    void clear();

    std::string getScheme() const;
    std::string getUsername() const;
    std::string getPassword() const;
    std::string getHost() const;
    unsigned short getPort() const;
    std::string getPath() const;
    std::string getQuery() const;
    std::string getFragment() const;

    std::string getFullPath() const; // path + query + fragment

    friend bool operator==(const CUrl& a, const CUrl& b);
    friend bool operator!=(const CUrl& a, const CUrl& b);
    friend bool operator<(const CUrl& a, const CUrl& b);

    void setSecure(bool secure);

    bool isIpv6() const;
    bool isSecure() const;

    std::string toString() const;
    // explicit operator std::string() const;

  protected:
    static bool unescape_path(const std::string& in, std::string& out);

    std::string_view captureUpTo(const std::string_view right_delimiter,
                                 const std::string& error_message = "");
    bool moveBefore(const std::string_view right_delimiter);
    bool existsForward(const std::string_view right_delimiter);

    std::string scheme;
    std::string authority;
    std::string user_info;
    std::string username;
    std::string password;
    std::string host;
    std::string port;
    std::string path;
    std::string query;
    std::string fragment;

    bool secure = false;
    bool ipv6_host = false;
    bool authority_present = false;

    std::string whole_url_storage;
    size_t left_position = 0;
    size_t right_position = 0;
    std::string_view parse_target;

  private:
    // clear all properties to valid empty values except whole_url_storage that
    // should be initialized as very first statement.
    void clearPrivate();
};

} // namespace upnplib

#endif // UPNPLIB_NET_URI_URL_HPP
