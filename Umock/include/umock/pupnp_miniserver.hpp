#ifndef UMOCK_PUPNP_MINISERVER_HPP
#define UMOCK_PUPNP_MINISERVER_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-19

// This is a header only mocking include file. When included it is present
// direct in the source code and can be used to mock static functions that are
// hidden to the global context.

#if UPNPLIB_WITH_NATIVE_PUPNP
static void RunMiniServer(MiniServerSockArray* miniSock);
#else
namespace {
void RunMiniServer(MiniServerSockArray* miniSock);
}
#endif

namespace umock {

class PupnpMiniServerInterface {
  public:
    virtual ~PupnpMiniServerInterface() = default;
    virtual void RunMiniServer(MiniServerSockArray* miniSock) = 0;
};


// This is the wrapper class for the real function
// -----------------------------------------------
class PupnpMiniServerReal : public PupnpMiniServerInterface {
  public:
    virtual ~PupnpMiniServerReal() override = default;
    void RunMiniServer(MiniServerSockArray* miniSock) override {
        return ::RunMiniServer(miniSock);
    }
};


// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    PupnpMiniServerReal pupnp_miniserver_realObj;            // already done below
    PupnpMiniServer pupnp_miniserver(&pupnp_miniserver_realObj); // already done below
    { // Other scope, e.g. within a gtest
        class PupnpMiniserverMock : public PupnpMiniServerInterface { ...; MOCK_METHOD(...) };
        PupnpMiniserverMock pupnp_miniserver_mockObj;
        PupnpMiniServer pupnp_httprw_injectObj(&pupnp_miniserver_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pupnp_miniserver_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default. */
// clang-format on
//------------------------------------------------------------------------------
class PupnpMiniServer {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    PupnpMiniServer(PupnpMiniServerReal* a_ptr_realObj) {
        m_ptr_workerObj = static_cast<PupnpMiniServerInterface*>(a_ptr_realObj);
    }

    // This constructor is used to inject the pointer to the mocking function.
    PupnpMiniServer(PupnpMiniServerInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is used to restore the old pointer.
    virtual ~PupnpMiniServer() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    virtual void RunMiniServer(MiniServerSockArray* miniSock) {
        return m_ptr_workerObj->RunMiniServer(miniSock);
    }

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of the class. With inline we do not need an extra definition line
    // outside the class.
    static inline PupnpMiniServerInterface* m_ptr_workerObj;
    PupnpMiniServerInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end. Because this is a header file the object
// must be static otherwise we will get a linker error of "multiple definition"
// when included in more than one source file.
static PupnpMiniServerReal pupnp_miniserver_realObj;
static PupnpMiniServer pupnp_miniserver(&pupnp_miniserver_realObj);

} // namespace umock

#endif // UMOCK_PUPNP_MINISERVER_HPP
