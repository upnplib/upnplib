// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-10

// Tools and helper classes to manage gtests
// =========================================

#include "upnplib_gtest_tools.hpp"

#include <iostream>
#include <cstring>
#include <fcntl.h> // Obtain O_* constant definitions

namespace upnplib {

// class CCaptureStdOutErr definition
// ----------------------------------
// We use a pipe that is opened non blocking.

CCaptureStdOutErr::CCaptureStdOutErr(int a_stdOutErrFd)
    : stdOutErrFd(a_stdOutErrFd), current_stdOutErrFd(a_stdOutErrFd) {

    if (this->stdOutErrFd != STDOUT_FILENO &&
        this->stdOutErrFd != STDERR_FILENO) {
        throw std::invalid_argument(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", constructor " + __func__ +
                        "(). Only STDOUT_FILENO and STDERR_FILENO supported."));
    }
    // make a pipe
#ifdef _WIN32
    int rc = ::_pipe(this->out_pipe, this->pipebuffer, _O_TEXT);
#else
    int rc = ::pipe(this->out_pipe);
#endif
    if (rc != 0)
        throw std::runtime_error(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ + "(). Failed to create a pipe. " +
            (std::string)std::strerror(errno) + '.'));

#ifndef _WIN32
    // Set non blocking mode on the pipe. read() shall not wait on an empty pipe
    // until it get some data but shall return immediately with errno = EAGAIN.
    rc = fcntl(this->out_pipe[0], F_SETFL, O_NONBLOCK);
    if (rc != 0)
        throw std::runtime_error(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ +
            "(). Failed to set non blocking mode on reading the pipe. " +
            (std::string)std::strerror(errno) + '.'));
#endif

    // save original stdout/stderr to restore after capturing
    this->orig_stdOutErrFd = ::dup(this->current_stdOutErrFd);

    if (this->orig_stdOutErrFd == -1)
        throw std::runtime_error(
            std::string((std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                        ", constructor " + __func__ +
                        "(). Failed to duplicate a file descriptor. " +
                        (std::string)std::strerror(errno) + '.'));
}

//
CCaptureStdOutErr::~CCaptureStdOutErr() {
    // Always restore original stdout/stderr file descriptor and close its
    // private duplicate.
    ::dup2(this->orig_stdOutErrFd, this->current_stdOutErrFd);
    ::close(this->orig_stdOutErrFd);

    // Close the pipe.
    ::close(this->out_pipe[0]);
    ::close(this->out_pipe[1]);
}

//
void CCaptureStdOutErr::start() {
    // redirect stdout/stderr to the pipe. The pipes write end now points to
    // stdout/stderr if using current_stdOutErrFd.
    if (::dup2(this->out_pipe[1], this->current_stdOutErrFd) == -1)

        throw std::runtime_error(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
            __func__ + "(). Failed to duplicate a file descriptor. " +
            (std::string)std::strerror(errno) + '.'));
}

//
std::string CCaptureStdOutErr::get() {
    // read from pipe into chunk and append the chunk to a string
    char chunk[this->chunk_size + 1];
    std::string strbuffer{};

    // Stdout is buffered. We need to flush it to the pipe. Otherwise it is
    // possible that we find an empty pipe.
    if (this->stdOutErrFd == STDOUT_FILENO)
        fflush(stdout);

    // Read pipe with chunks
    ssize_t count{2};
    while (count > 1) {

        // We always write a nullbyte to the pipe so read always returns
        // and does not block if there is nothing captured.
        constexpr char nullbyte[1]{};
        if (::write(this->out_pipe[1], &nullbyte, 1) == -1)

            throw std::runtime_error(std::string(
                (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
                __func__ + "(). Failed to write to the pipe. " +
                (std::string)std::strerror(errno) + '.'));

        // Read from the pipe
        memset(&chunk, 0, sizeof(chunk));
        count = ::read(this->out_pipe[0], &chunk, this->chunk_size);

        switch (count) {
        case 1:
            // Here we have read from an empty pipe containing only the written
            // null byte. It is the normal end condition of the while loop.
            break;

        case -1:
            if (errno == EAGAIN)
                // EAGAIN(11): "Resource temporarily unavailable"
                // means nothing to read, the pipe is empty.
                // It is only returned in non blocking mode.
                // This is also one normal end condition of the while loop.
                // With always writing a null byte this case should never match.
                // But I want to have it available for documentation and
                // possible reuse.
                break;
            else
                throw std::runtime_error(std::string(
                    (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                    ", " + __func__ + "(). Failed to read from pipe. " +
                    (std::string)std::strerror(errno) + '.'));

        case 0:
            throw std::runtime_error(std::string(
                (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
                __func__ + "(). Read 0 byte from pipe. " +
                (std::string)std::strerror(errno) + '.'));

        default:
            if (chunk[0] == '\0') {
                // Here we got an empty string but with more than one null byte.
                // We will finish reading.
                count = 1;
                break;
            }
            // Continue reading next chunk from the pipe
            strbuffer += chunk;
            break;
        }
    }

    // reconnect stdout/stderr to original system output.
    if (::dup2(this->orig_stdOutErrFd, this->current_stdOutErrFd) == -1)

        throw std::runtime_error(std::string(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
            __func__ + "(). Failed to duplicate a file descriptor. " +
            (std::string)std::strerror(errno) + '.'));

    return strbuffer;
}

} // namespace upnplib
