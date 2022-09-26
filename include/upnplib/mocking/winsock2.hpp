#ifndef UPNPLIB_MOCKING_WINSOCK2_HPP
#define UPNPLIB_MOCKING_WINSOCK2_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-26

// winsock2.h is a Microsoft Windows library.

#include "upnplib/visibility.hpp"
#include <winsock2.h>

namespace upnplib {
namespace mocking {

class Winsock2Interface {
  public:
    virtual ~Winsock2Interface() = default;
    virtual int WSAGetLastError() = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class Winsock2Real : public Winsock2Interface {
  public:
    virtual ~Winsock2Real() override = default;
    int WSAGetLastError() override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    Winsock2Real Winsock2_realObj;
    Winsock2(&net_if_realObj;
    { // Other scope, e.g. within a gtest
        class Winsock2Mock { ...; MOCK_METHOD(...) };
        Winsock2Mock net_if_mockObj;
        Winsock2(&winsock2_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class UPNPLIB_API Winsock2 {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Winsock2(Winsock2Real* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Winsock2(Winsock2Interface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Winsock2();

    virtual int WSAGetLastError();

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Winsock2::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline Winsock2Interface* m_ptr_workerObj;
    Winsock2Interface* m_ptr_oldObj{};
};

extern Winsock2 UPNPLIB_API winsock2_h;

} // namespace mocking
} // namespace upnplib

#endif // UPNPLIB_MOCKING_WINSOCK2_HPP
