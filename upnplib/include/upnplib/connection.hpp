#ifndef UPNPP_0_ADDRESSING_CONNECTION_HPP
#define UPNPP_0_ADDRESSING_CONNECTION_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-16

#ifndef _MSC_VER

#include <upnplib/visibility.hpp>
#include <upnplib/socket.hpp>
#include <csignal> // not usable on win32


namespace upnplib {

// Taking the idea and example for this SIGPIPE handler from
// REF:_[SIGPIPE_handler](https://stackoverflow.com/a/2347848/5014688) and
// adapt it. Thanks to kroki.
// This is a general solution, for when you are not in full control of the
// program’s signal handling and want to write data to an actual pipe or use
// write(2) on a socket. To control SIGPIPE on send(2) There can be used option
// MSG_NOSIGNAL on Linux and SO_NOSIGPIPE on MacOS. But with SSL function calls
// we have to use this class because it does not use syscall send() but write()
// and that doesn't have the options. To simplify things I will use this
// solution on all supported platforms.
// This is a scoped version just using the constructor and destructor. The last
// version with methods 'suppress()' and 'restore()' you find at commit
// c76d123ffbd6989ff415169b86fe0b27c5a111aa. --Ingo
class UPNPLIB_API CSigpipe_scoped {
  public:
    // The only error that can be returned in errno is 'EINVAL signum is not a
    // valid signal.' SIGPIPE is always a valid signum, so no error handling is
    // required.
    CSigpipe_scoped();
    ~CSigpipe_scoped();

  private:
    sigset_t m_sigpipe_mask;
    bool m_sigpipe_pending;
    bool m_sigpipe_unblock;
};

} // namespace upnplib

#define UPNPLIB_SCOPED_NO_SIGPIPE upnplib::CSigpipe_scoped sigpipe
#else // #ifndef _MSC_VER
#define UPNPLIB_SCOPED_NO_SIGPIPE
#endif

#endif // UPNPP_0_ADDRESSING_CONNECTION_HPP
