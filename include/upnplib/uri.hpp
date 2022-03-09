// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-29

#ifndef INCLUDE_UPNPLIB_URI_HPP
#define INCLUDE_UPNPLIB_URI_HPP

#include "pupnp/upnp/src/inc/uri.hpp"

namespace upnplib {

//
// Interface for the uri module
// ============================
// clang-format off

class Iuri {
  public:
    virtual ~Iuri() {}

    virtual int replace_escaped(
            char* in, size_t index, size_t* max) = 0;
    virtual int copy_URL_list(
            URL_list* in, URL_list* out) = 0;
    virtual void free_URL_list(
            URL_list* list) = 0;
    virtual int token_string_casecmp(
            token* in1, const char* in2) = 0;
    virtual int token_cmp(
            token* in1, token* in2) = 0;
    virtual int remove_escaped_chars(
            char* in, size_t* size) = 0;
    virtual int remove_dots(
            char* buf, size_t size) = 0;
    virtual char* resolve_rel_url(
            char* base_url, char* rel_url) = 0;
    virtual int parse_uri(
            const char *in, size_t max, uri_type *out) = 0;
};

class Curi : Iuri {
  public:
    virtual ~Curi() override {}

    int replace_escaped(char* in, size_t index, size_t* max) override {
        return ::replace_escaped(in, index, max); }
    int copy_URL_list(URL_list* in, URL_list* out) override {
        return ::copy_URL_list(in, out); }
    void free_URL_list(URL_list* list) override {
        return ::free_URL_list(list); }
    int token_string_casecmp(token* in1, const char* in2) override {
        return ::token_string_casecmp(in1, in2); }
    int token_cmp(token* in1, token* in2) override {
        return ::token_cmp(in1, in2); }
    int remove_escaped_chars(char* in, size_t* size) override {
        return ::remove_escaped_chars(in, size); }
    int remove_dots(char* buf, size_t size) override {
        return ::remove_dots(buf, size); }
    char* resolve_rel_url(char* base_url, char* rel_url) override {
        return ::resolve_rel_url(base_url, rel_url); }
    int parse_uri(const char *in, size_t max, uri_type *out) override {
        return ::parse_uri(in, max, out); }
};
// clang-format on

//
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
    CUrl();
    CUrl(const std::string& s);

    std::string getScheme() const;
    std::string getUsername() const;
    std::string getPassword() const;
    std::string getHost() const;
    unsigned short getPort() const;
    std::string getPath() const;
    std::string getQuery() const;
    const std::multimap<std::string, std::string>& getQueryParameters() const;
    std::string getFragment() const;

    std::string getFullPath() const; // path + query + fragment

    void fromString(const std::string& s);

    friend bool operator==(const CUrl& a, const CUrl& b);
    friend bool operator!=(const CUrl& a, const CUrl& b);
    friend bool operator<(const CUrl& a, const CUrl& b);

    void setSecure(bool secure);

    bool isIpv6() const;
    bool isSecure() const;

    std::string toString() const;
    explicit operator std::string() const;

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
    std::multimap<std::string, std::string> query_parameters;
    std::string fragment;

    bool secure = false;
    bool ipv6_host = false;
    bool authority_present = false;

    std::string whole_url_storage;
    size_t left_position = 0;
    size_t right_position = 0;
    std::string_view parse_target;
};

} // namespace upnplib

#endif // INCLUDE_UPNPLIB_URI_HPP
