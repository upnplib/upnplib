#ifndef UPNPLIB_NET_URI_HPP
#define UPNPLIB_NET_URI_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-25
/*!
 * \file
 * \brief C++ interface for the Uri module.
 */

/// \cond
namespace upnplib {


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

} // namespace upnplib
/// \endcond
#endif // UPNPLIB_NET_URI_HPP
