#ifndef UMOCK_SYSINFO_HPP
#define UMOCK_SYSINFO_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include "upnplib/visibility.hpp"
#include <ctime>
#ifndef _WIN32
#include <sys/utsname.h>
#endif

namespace umock {

class UPNPLIB_API SysinfoInterface {
  public:
    SysinfoInterface();
    virtual ~SysinfoInterface();
    virtual time_t time(time_t* tloc) = 0;
#ifndef _WIN32
    virtual int uname(utsname* buf) = 0;
#endif
};

//
// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class SysinfoReal : public SysinfoInterface {
  public:
    SysinfoReal();
    virtual ~SysinfoReal() override;
    time_t time(time_t* tloc) override;
#ifndef _WIN32
    int uname(utsname* buf) override;
#endif
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    SysinfoReal sysinfo_realObj;       // already done
    Sysinfo sysinfo(&sysinfo_realObj); // already done
    { // Other scope, e.g. within a gtest
        class SysinfoMock : public SysinfoInterface { ...; MOCK_METHOD(...) };
        SysinfoMock sysinfo_mockObj;
        Sysinfo sysinfo_injectObj(&string_mockObj); // obj. name doesn't matter
        EXPECT_CALL(sysinfo_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Sysinfo {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Sysinfo(SysinfoReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Sysinfo(SysinfoInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Sysinfo();

    // Methods
    virtual time_t time(time_t* tloc);
#ifndef _WIN32
    virtual int uname(utsname* buf);
#endif

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Sysinfo::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline SysinfoInterface* m_ptr_workerObj;
    SysinfoInterface* m_ptr_oldObj{};
};


UPNPLIB_EXTERN Sysinfo sysinfo;

} // namespace umock

#endif // UMOCK_SYSINFO_HPP
