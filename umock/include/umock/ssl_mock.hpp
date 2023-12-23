#ifdef UPNP_ENABLE_OPEN_SSL
#ifndef UMOCK_SSL_MOCK_HPP
#define UMOCK_SSL_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-23

#include <umock/ssl.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API SslMock : public umock::SslInterface {
  public:
    SslMock();
    virtual ~SslMock() override;
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(int, SSL_read, (SSL * ssl, void* buf, int num), (override));
    ENABLE_MSVC_WARN
};

} // namespace umock

#endif // UMOCK_SSL_MOCK_HPP
#endif // UPNP_ENABLE_OPEN_SSL
