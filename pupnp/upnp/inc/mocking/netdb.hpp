#ifndef MOCKING_NETDB_HPP
#define MOCKING_NETDB_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-20

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h> // important for struct addrinfo
#endif

namespace mocking {

class NetdbInterface {
  public:
    virtual ~NetdbInterface() {}
    virtual int getaddrinfo(const char* node, const char* service,
                            const struct addrinfo* hints,
                            struct addrinfo** res) = 0;
    virtual void freeaddrinfo(struct addrinfo* res) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class NetdbReal : public NetdbInterface {
  public:
    virtual ~NetdbReal() override {}
    virtual int getaddrinfo(const char* node, const char* service,
                            const struct addrinfo* hints,
                            struct addrinfo** res) override;
    virtual void freeaddrinfo(struct addrinfo* res) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    NetdbReal netdb_realObj;
    Netdb(&netdb_realObj;
    { // Other scope, e.g. within a gtest
        class NetdbMock { ...; MOCK_METHOD(...) };
        NetdbMock netdb_mockObj;
        Netdb(&netdb_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class EXPORT_SPEC Netdb {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Netdb(NetdbReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Netdb(NetdbInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Netdb();

    int getaddrinfo(const char* node, const char* service,
                    const struct addrinfo* hints, struct addrinfo** res);
    void freeaddrinfo(struct addrinfo* res);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Netdb::m_ptr_workerObj.
    EXPORT_SPEC_LOCAL static inline NetdbInterface* m_ptr_workerObj;
    NetdbInterface* m_ptr_oldObj{};
};

extern Netdb EXPORT_SPEC netdb_h;

} // namespace mocking

#endif // MOCKING_NETDB_HPP
