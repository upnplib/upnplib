// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

// Tools and helper classes to manage gtests
// =========================================

#include "upnplib/port.hpp"
#include "upnplib/gtest.hpp"

#ifdef _WIN32
#include <ws2tcpip.h> // for socklen_t etc.
#else
#include <sys/socket.h>
#endif

#include <iostream>
#include <cstring>
#include <fcntl.h> // Obtain O_* constant definitions

namespace upnplib::testing {

// class CaptureStdOutErr definition
// ----------------------------------
// We use a pipe that is opened non blocking.

CaptureStdOutErr::CaptureStdOutErr(int a_stdOutErrFd)
    : stdOutErrFd(a_stdOutErrFd), current_stdOutErrFd(a_stdOutErrFd) {

    if (this->stdOutErrFd != STDOUT_FILENO &&
        this->stdOutErrFd != STDERR_FILENO) {
        throw std::invalid_argument(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ +
            "(). Only STDOUT_FILENO and STDERR_FILENO supported.");
    }
    // make a pipe
#ifdef _WIN32
    int rc = ::_pipe(this->out_pipe, m_pipebuffer, _O_TEXT);
#else
    int rc = ::pipe(this->out_pipe);
#endif
    if (rc != 0)
        throw std::runtime_error((std::string)__FILE__ + ":" +
                                 std::to_string(__LINE__) + ", constructor " +
                                 __func__ + "(). Failed to create a pipe. " +
                                 (std::string)std::strerror(errno) + '.');

#ifndef _WIN32
    // Set non blocking mode on the pipe. read() shall not wait on an empty pipe
    // until it get some data but shall return immediately with errno = EAGAIN.
    rc = fcntl(this->out_pipe[0], F_SETFL, O_NONBLOCK);
    if (rc != 0)
        throw std::runtime_error(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
            ", constructor " + __func__ +
            "(). Failed to set non blocking mode on reading the pipe. " +
            (std::string)std::strerror(errno) + '.');
#endif

    // save original stdout/stderr to restore after capturing
    this->orig_stdOutErrFd = ::dup(this->current_stdOutErrFd);

    if (this->orig_stdOutErrFd == -1)
        throw std::runtime_error((std::string)__FILE__ + ":" +
                                 std::to_string(__LINE__) + ", constructor " +
                                 __func__ +
                                 "(). Failed to duplicate a file descriptor. " +
                                 (std::string)std::strerror(errno) + '.');
}

//
CaptureStdOutErr::~CaptureStdOutErr() {
    // Always restore original stdout/stderr file descriptor and close its
    // private duplicate.
    ::dup2(this->orig_stdOutErrFd, this->current_stdOutErrFd);
    ::close(this->orig_stdOutErrFd);

    // Close the pipe.
    ::close(this->out_pipe[0]);
    ::close(this->out_pipe[1]);
}

//
void CaptureStdOutErr::start() {
    // redirect stdout/stderr to the pipe. The pipes write end now points to
    // stdout/stderr if using current_stdOutErrFd.
    if (::dup2(this->out_pipe[1], this->current_stdOutErrFd) == -1)

        throw std::runtime_error((std::string)__FILE__ + ":" +
                                 std::to_string(__LINE__) + ", " + __func__ +
                                 "(). Failed to duplicate a file descriptor. " +
                                 (std::string)std::strerror(errno) + '.');
}

//
std::string CaptureStdOutErr::get() {
    // read from pipe into chunk and append the chunk to a string
    char chunk[m_chunk_size + 1];
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

            throw std::runtime_error(
                (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
                __func__ + "(). Failed to write to the pipe. " +
                (std::string)std::strerror(errno) + '.');

        // Read from the pipe
        memset(&chunk, 0, sizeof(chunk));
        count = ::read(this->out_pipe[0], &chunk, m_chunk_size);

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
                throw std::runtime_error(
                    (std::string)__FILE__ + ":" + std::to_string(__LINE__) +
                    ", " + __func__ + "(). Failed to read from pipe. " +
                    (std::string)std::strerror(errno) + '.');

        case 0:
            throw std::runtime_error((std::string)__FILE__ + ":" +
                                     std::to_string(__LINE__) + ", " +
                                     __func__ + "(). Read 0 byte from pipe. " +
                                     (std::string)std::strerror(errno) + '.');

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

        throw std::runtime_error((std::string)__FILE__ + ":" +
                                 std::to_string(__LINE__) + ", " + __func__ +
                                 "(). Failed to duplicate a file descriptor. " +
                                 (std::string)std::strerror(errno) + '.');

    return strbuffer;
}

//
// function to get the modification time of a file
// -----------------------------------------------
time_t file_mod_time(const std::string& a_pathname) {
    std::cout << "DEBUG! Tracepoint 2\n";
    struct stat result;

    if (stat(a_pathname.c_str(), &result) == -1)
        throw std::invalid_argument(
            (std::string)__FILE__ + ":" + std::to_string(__LINE__) + ", " +
            __func__ + "(). Failed to get modification time of \"" +
            a_pathname + "\". " + std::strerror(errno) + '.');

    return result.st_mtime;
}

//
// function to test if file descriptors are closed
// -----------------------------------------------
void check_closed_fds(int a_from_fd, int a_to_fd) {
    char error_buffer[64];
    socklen_t sockerrlen = sizeof(error_buffer);

    for (int i = a_from_fd; i <= a_to_fd; i++) {

        // Throw an error if we find an open file descriptor
        errno = 0;
        int rc =
            getsockopt(i, SOL_SOCKET, SO_ERROR, &error_buffer[0], &sockerrlen);
        if ((rc == 0) || (rc == -1 && errno == 88) ||
            (rc == -1 && errno != 9)) {
            // errno  9: "Bad file descriptor" means no open fd -> no error
            // errno 88: "Socket operation on non-socket" means open fd -> error

            throw std::runtime_error(
                "Found open file descriptor " + std::to_string(i) + ". (" +
                std::to_string(errno) + ") " + std::strerror(errno));
        }
    }
}

} // namespace upnplib::testing
