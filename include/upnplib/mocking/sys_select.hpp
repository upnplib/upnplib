#ifndef MOCKING_SYS_SELECT_HPP
#define MOCKING_SYS_SELECT_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-27

#include "upnplib/visibility.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

namespace upnplib {
namespace mocking {

class Sys_selectInterface {
  public:
    virtual ~Sys_selectInterface() {}
    virtual int select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class Sys_selectReal : public Sys_selectInterface {
  public:
    virtual ~Sys_selectReal() override {}
    int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
               struct timeval* timeout) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    Sys_selectReal sys_select_realObj; // already done below
    Sys_select(&sys_select_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class Sys_selectMock : public Sys_selectInterface { ...; MOCK_METHOD(...) };
        Sys_selectMock sys_select_mockObj;
        Sys_select sys_select_injectObj(&sys_select_mockObj); // obj. name doesn't matter
        EXPECT_CALL(sys_select_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
// clang-format on
class UPNPLIB_API Sys_select {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Sys_select(Sys_selectReal* a_ptr_mockObj);

    // This constructor is used to inject the pointer to the mocking function.
    Sys_select(Sys_selectInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Sys_select();

    // Methods
    virtual int select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Sys_select::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline Sys_selectInterface* m_ptr_workerObj;
    Sys_selectInterface* m_ptr_oldObj{};
};

extern Sys_select UPNPLIB_API sys_select_h;

} // namespace mocking
} // namespace upnplib

#endif // MOCKING_SYS_SELECT_HPP
