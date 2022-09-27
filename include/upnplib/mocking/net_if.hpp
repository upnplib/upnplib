#ifndef UPNPLIB_MOCKING_NET_IF_HPP
#define UPNPLIB_MOCKING_NET_IF_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-25

#include "upnplib/visibility.hpp"
#include <net/if.h>

namespace upnplib {
namespace mocking {

class Net_ifInterface {
  public:
    virtual ~Net_ifInterface() = default;
    virtual unsigned int if_nametoindex(const char* ifname) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class Net_ifReal : public Net_ifInterface {
  public:
    virtual ~Net_ifReal() override = default;
    unsigned int if_nametoindex(const char* ifname) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    Net_ifReal net_if_realObj;
    Net_if(&net_if_realObj;
    { // Other scope, e.g. within a gtest
        class Net_ifMock { ...; MOCK_METHOD(...) };
        Net_ifMock net_if_mockObj;
        Net_if(&net_if_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class UPNPLIB_API Net_if {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Net_if(Net_ifReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Net_if(Net_ifInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Net_if();

    virtual unsigned int if_nametoindex(const char* ifname);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Net_if::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline Net_ifInterface* m_ptr_workerObj;
    Net_ifInterface* m_ptr_oldObj{};
};

extern Net_if UPNPLIB_API net_if_h;

} // namespace mocking
} // namespace upnplib

#endif // UPNPLIB_MOCKING_NET_IF_HPP