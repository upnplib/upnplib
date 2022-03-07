// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-07

// Tools and helper classes to manage gtests
// =========================================

#include "upnplib_gtest_tools.hpp"

#include <iostream>
#include <string.h>

namespace upnplib {

// class CCaptureStdOutErr definition
// ----------------------------------
CCaptureStdOutErr::CCaptureStdOutErr(int a_fileno) {
    if (a_fileno != STDOUT_FILENO && a_fileno != STDERR_FILENO) {
        throw ::std::invalid_argument(::std::string(
            (::std::string)__FILE__ + ":" + ::std::to_string(__LINE__) +
            ", constructor " + __func__ +
            ". Only STDOUT_FILENO and STDERR_FILENO supported. "
            "Nothing will be captured."));
    }
    // make a pipe
#ifdef _WIN32
    int rc = ::_pipe(m_out_pipe, m_chunk_size, O_TEXT);
#else
    int rc = ::pipe(m_out_pipe);
#endif
    if (rc != 0) {
        throw ::std::runtime_error(::std::string(
            (::std::string)__FILE__ + ":" + ::std::to_string(__LINE__) +
            ", constructor " + __func__ + ". Creating a pipe failed. " +
            (::std::string)strerror(errno) + '.'));
    }
    m_std_fileno = a_fileno;
    m_saved_stdno =
        ::dup(a_fileno); // save stdout/stderr to restore after capturing
}

//
CCaptureStdOutErr::~CCaptureStdOutErr() {
    ::close(m_out_pipe[0]);
    ::close(m_out_pipe[1]);
}

//
void CCaptureStdOutErr::start() {
    ::dup2(m_out_pipe[1], m_std_fileno); // redirect stdout/stderr to the pipe
}

//
::std::string CCaptureStdOutErr::get() {
    // We always write a nullbyte to the pipe so read always returns
    // and does not wait endless if there is nothing captured.
    const char nullbyte[1] = {'\0'};
    if (::write(m_std_fileno, &nullbyte, 1) == -1)
        return "";

    // read from pipe into chunk and append the chunk to a string
    char chunk[m_chunk_size];
    ::std::string strbuffer{};
    while (::read(m_out_pipe[0], &chunk, m_chunk_size - 1) > 1) {
        strbuffer += chunk;
        if (::write(m_std_fileno, &nullbyte, 1) == -1)
            break;
    }

    // reconnect stdout/stderr
    ::dup2(m_saved_stdno, m_std_fileno);

    return strbuffer;
}

} // namespace upnplib
