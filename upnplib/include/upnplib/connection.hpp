#ifndef UPNPP_0_ADDRESSING_CONNECTION_HPP
#define UPNPP_0_ADDRESSING_CONNECTION_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-07

#include <upnplib/visibility.hpp>
#include <upnplib/socket.hpp>

#ifndef _WIN32
#include <csignal>
#endif


namespace upnplib {

// Taking the idea and example for this SIGPIPE handler from
// REF:_[SIGPIPE_handler](https://stackoverflow.com/a/2347848/5014688) and
// adapt it. Thanks to kroki.
// This is a general solution, for when you are not in full control of the
// program’s signal handling and want to write data to an actual pipe or use
// write(2) on a socket. To control SIGPIPE on send(2) There can be used option
// MSG_NOSIGNAL on Linux and SO_NOSIGPIPE on MacOS. But with SSL function calls
// we have to use this class because it does not use syscall send() but write()
// and that doesn't have the options. --Ingo
class UPNPLIB_API CSigpipe {
#if !defined __APPLE__ && !defined _MSC_VER
  private:
    ::sigset_t m_sigpipe_mask;
    bool m_sigpipe_pending;
    bool m_sigpipe_unblock;

  public:
    // The only error that can be returned in errno is 'EINVAL signum is not a
    // valid signal.' SIGPIPE is always a valid signum, so no error handling is
    // required.
    CSigpipe();
#endif
  public:
    void suppress([[maybe_unused]] SOCKET sockfd);
    void restore();
};

} // namespace upnplib

#endif // UPNPP_0_ADDRESSING_CONNECTION_HPP
