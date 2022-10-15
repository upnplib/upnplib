// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-16

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

// winsock2.h is a Microsoft Windows library.

#include "upnplib/visibility.hpp"
#include <winsock2.h>

namespace upnplib::mocking {

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
// clang-format off
/* Example:
    Winsock2Real winsock2_realObj; // already done below
    Winsock2(&winsock2_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class Winsock2Mock : public Winsock2Interface { ...; MOCK_METHOD(...) };
        Winsock2Mock winsock2_mockObj;
        Winsock2 winsock2_injectObj(&winsock2_mockObj); // obj. name doesn't matter
        EXPECT_CALL(winsock2_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
// clang-format on
class UPNPLIB_API Winsock2 {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Winsock2(Winsock2Real* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Winsock2(Winsock2Interface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Winsock2();

    // Methods
    virtual int WSAGetLastError();

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Winsock2::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline Winsock2Interface* m_ptr_workerObj;
    Winsock2Interface* m_ptr_oldObj{};
};

} // namespace upnplib::mocking
