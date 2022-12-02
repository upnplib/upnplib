// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

#include "upnplib/visibility.hpp"
#include <ifaddrs.h>

namespace umock {

class IfaddrsInterface {
  public:
    virtual ~IfaddrsInterface() = default;

    virtual int getifaddrs(struct ifaddrs** ifap) = 0;
    virtual void freeifaddrs(struct ifaddrs* ifa) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class IfaddrsReal : public IfaddrsInterface {
  public:
    virtual ~IfaddrsReal() override = default;

    int getifaddrs(struct ifaddrs** ifap) override;
    void freeifaddrs(struct ifaddrs* ifa) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    IfaddrsReal ifaddrs_realObj; // already done below
    Ifaddrs(&ifaddrs_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class IfaddrsMock : public IfaddrsInterface { ...; MOCK_METHOD(...) };
        IfaddrsMock ifaddrs_mockObj;
        Ifaddrs ifaddrs_injectObj(&ifaddrs_mockObj); // obj. name doesn't matter
        EXPECT_CALL(ifaddrs_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //----------------------------------------------------------------------------
// clang-format on
class UPNPLIB_API Ifaddrs {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Ifaddrs(IfaddrsReal* a_ptr_mockObj);

    // This constructor is used to inject the pointer to the mocking function.
    Ifaddrs(IfaddrsInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Ifaddrs();

    // Methods
    virtual int getifaddrs(struct ifaddrs** ifap);
    virtual void freeifaddrs(struct ifaddrs* ifa);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Ifaddrs::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline IfaddrsInterface* m_ptr_workerObj;
    IfaddrsInterface* m_ptr_oldObj{};
};

} // namespace umock
// vim: syntax=cpp
