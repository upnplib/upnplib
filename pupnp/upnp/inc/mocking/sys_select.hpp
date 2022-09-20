#ifndef MOCKING_SYS_SELECT_HPP
#define MOCKING_SYS_SELECT_HPP
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-20

#include "UpnpGlobal.hpp" // for EXPORT_SPEC

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

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
    virtual int select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    Sys_selectReal sys_select_realObj;
    Sys_select(&sys_select_realObj;
    { // Other scope, e.g. within a gtest
        class Sys_selectMock { ...; MOCK_METHOD(...) };
        Sys_selectMock sys_select_mockObj;
        Sys_select(&sys_select_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class EXPORT_SPEC Sys_select {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Sys_select(Sys_selectReal* a_ptr_mockObj);

    // This constructor is used to inject the pointer to the mocking function.
    Sys_select(Sys_selectInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Sys_select();

    int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
               struct timeval* timeout);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Sys_select::m_ptr_workerObj.
    EXPORT_SPEC_LOCAL static inline Sys_selectInterface* m_ptr_workerObj;
    Sys_selectInterface* m_ptr_oldObj{};
};

extern Sys_select EXPORT_SPEC sys_select_h;

} // namespace mocking

#endif // MOCKING_SYS_SELECT_HPP
