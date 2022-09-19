#ifndef MOCKWRAP_IFADDRS_HPP
#define MOCKWRAP_IFADDRS_HPP
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-19

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#include <ifaddrs.h>

namespace mockwrap {

class IfaddrsInterface {
  public:
    virtual ~IfaddrsInterface() {}

    virtual int getifaddrs(struct ifaddrs** ifap) = 0;
    virtual void freeifaddrs(struct ifaddrs* ifa) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class IfaddrsReal : public IfaddrsInterface {
  public:
    virtual ~IfaddrsReal() override {}

    virtual int getifaddrs(struct ifaddrs** ifap) override;
    virtual void freeifaddrs(struct ifaddrs* ifa) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    IfaddrsReal ifaddrs_realObj;
    Ifaddrs(&ifaddrs_realObj;
    { // Other scope, e.g. within a gtest
        class IfaddrsMock { ...; MOCK_METHOD(...) };
        IfaddrsMock ifaddrs_mockObj;
        Ifaddrs(&ifaddrs_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class EXPORT_SPEC Ifaddrs {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Ifaddrs(IfaddrsReal* a_ptr_mockObj);

    // This constructor is used to inject the pointer to the mocking function.
    Ifaddrs(IfaddrsInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Ifaddrs();

    int getifaddrs(struct ifaddrs** ifap);
    void freeifaddrs(struct ifaddrs* ifa);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Ifaddrs::m_ptr_workerObj.
    EXPORT_SPEC_LOCAL static inline IfaddrsInterface* m_ptr_workerObj;
    IfaddrsInterface* m_ptr_oldObj{};
};

extern Ifaddrs EXPORT_SPEC ifaddrs_h;

} // namespace mockwrap

#endif // MOCKWRAP_IFADDRS_HPP
