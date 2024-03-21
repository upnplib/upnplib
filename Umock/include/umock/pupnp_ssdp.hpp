#if defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP) ||      \
    defined(INCLUDE_CLIENT_APIS) || defined(INCLUDE_DEVICE_APIS)

#ifndef UMOCK_PUPNP_SSDP_HPP
#define UMOCK_PUPNP_SSDP_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-21

// This is a header only mocking include file. When included it is present
// direct in the source code and can be used to mock static functions that are
// hidden to the global context.

int get_ssdp_sockets(MiniServerSockArray* out);

namespace umock {

class PupnpSsdpInterface {
  public:
    virtual ~PupnpSsdpInterface() = default;
    virtual int get_ssdp_sockets(MiniServerSockArray* out) = 0;
};


// This is the wrapper class for the real function
// -----------------------------------------------
class PupnpSsdpReal : public PupnpSsdpInterface {
  public:
    virtual ~PupnpSsdpReal() override = default;
    int get_ssdp_sockets(MiniServerSockArray* out) override {
        return ::get_ssdp_sockets(out);
    }
};


// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    PupnpSsdpReal pupnp_ssdp_realObj;            // already done below
    PupnpSsdp pupnp_ssdp(&pupnp_ssdp_realObj); // already done below
    { // Other scope, e.g. within a gtest
        class PupnpSsdpMock : public PupnpSsdpInterface { ...; MOCK_METHOD(...) };
        PupnpSsdpMock pupnp_ssdp_mockObj;
        PupnpSsdp pupnp_ssdp_injectObj(&pupnp_ssdp_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pupnp_ssdp_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default. */
// clang-format on
//------------------------------------------------------------------------------
class PupnpSsdp {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    PupnpSsdp(PupnpSsdpReal* a_ptr_realObj) {
        m_ptr_workerObj = static_cast<PupnpSsdpInterface*>(a_ptr_realObj);
    }

    // This constructor is used to inject the pointer to the mocking function.
    PupnpSsdp(PupnpSsdpInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is used to restore the old pointer.
    virtual ~PupnpSsdp() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    virtual int get_ssdp_sockets(MiniServerSockArray* out) {
        return m_ptr_workerObj->get_ssdp_sockets(out);
    }

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of the class. With inline we do not need an extra definition line
    // outside the class.
    static inline PupnpSsdpInterface* m_ptr_workerObj;
    PupnpSsdpInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
inline PupnpSsdpReal pupnp_ssdp_realObj;
inline PupnpSsdp pupnp_ssdp(&pupnp_ssdp_realObj);

} // namespace umock

#endif // UMOCK_PUPNP_SSDP_HPP
#endif // defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP)
