#ifndef MOCK_NET_IF_HPP
#define MOCK_NET_IF_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-23

#include <upnplib/visibility.hpp>
#include <net/if.h>

namespace umock {

class UPNPLIB_API Net_ifInterface {
  public:
    Net_ifInterface();
    virtual ~Net_ifInterface();
    virtual unsigned int if_nametoindex(const char* ifname) = 0;
};

//
// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class Net_ifReal : public Net_ifInterface {
  public:
    Net_ifReal();
    virtual ~Net_ifReal() override;
    unsigned int if_nametoindex(const char* ifname) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    Net_ifReal net_if_realObj; // already done below
    Net_if(&net_if_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class Net_ifMock : public Net_ifInterface { ...; MOCK_METHOD(...) };
        Net_ifMock net_if_mockObj;
        Net_if net_if_injectObj(&net_if_mockObj); // obj. name doesn't matter
        EXPECT_CALL(net_if_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Net_if {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Net_if(Net_ifReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Net_if(Net_ifInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Net_if();

    // Methods
    virtual unsigned int if_nametoindex(const char* ifname);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Net_if::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline Net_ifInterface* m_ptr_workerObj;
    Net_ifInterface* m_ptr_oldObj{};
};


UPNPLIB_EXTERN Net_if net_if_h;

} // namespace umock

#endif // MOCK_NET_IF_HPP
