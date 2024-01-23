#ifdef UPNP_ENABLE_OPEN_SSL
#ifndef UMOCK_SSL_HPP
#define UMOCK_SSL_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-27

#include <upnplib/visibility.hpp>
#include <openssl/ssl.h>

namespace umock {

class UPNPLIB_API SslInterface {
  public:
    SslInterface();
    virtual ~SslInterface();
    virtual int SSL_read(SSL* ssl, void* buf, int num) = 0;
    virtual int SSL_write(SSL* ssl, const void* buf, int num) = 0;
};


// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class SslReal : public SslInterface {
  public:
    SslReal();
    virtual ~SslReal() override;
    int SSL_read(SSL* ssl, void* buf, int num) override;
    int SSL_write(SSL* ssl, const void* buf, int num) override;
};


// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    SslReal ssl_realObj;     // already done
    Ssl ssl_h(&ssl_realObj); // already done
    { // Other scope, e.g. within a gtest
        class SslMock : public SslInterface { ...; MOCK_METHOD(...) };
        SslMock ssl_mockObj;
        Ssl ssl_injectObj(&ssl_mockObj); // obj. name doesn't matter
        EXPECT_CALL(ssl_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Ssl {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Ssl(SslReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Ssl(SslInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Ssl();

    // Methods
    virtual int SSL_read(SSL* ssl, void* buf, int num);
    virtual int SSL_write(SSL* ssl, const void* buf, int num);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Ssl::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline SslInterface* m_ptr_workerObj;
    SslInterface* m_ptr_oldObj{};
};


UPNPLIB_EXTERN Ssl ssl_h;

} // namespace umock

#endif // UMOCK_SSL_HPP
#endif // UPNP_ENABLE_OPEN_SSL
