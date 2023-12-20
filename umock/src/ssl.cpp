// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-22

#include <umock/ssl.hpp>
#include <upnplib/port.hpp>

namespace umock {

SslInterface::SslInterface() = default;
SslInterface::~SslInterface() = default;

SslReal::SslReal() = default;
SslReal::~SslReal() = default;
int SslReal::SSL_read(SSL* ssl, void* buf, int num) {
    return ::SSL_read(ssl, buf, num);
}

// This constructor is used to inject the pointer to the real function.
Ssl::Ssl(SslReal* a_ptr_realObj) {
    m_ptr_workerObj = (SslInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Ssl::Ssl(SslInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Ssl::~Ssl() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Ssl::SSL_read(SSL* ssl, void* buf, int num) {
    return m_ptr_workerObj->SSL_read(ssl, buf, num);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
SslReal ssl_realObj;
SUPPRESS_MSVC_WARN_4273_NEXT_LINE
UPNPLIB_API Ssl ssl_h(&ssl_realObj);

} // namespace umock
