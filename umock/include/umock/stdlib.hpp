#ifndef UMOCK_STDLIB_HPP
#define UMOCK_STDLIB_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <upnplib/visibility.hpp>
#include <stdlib.h>

namespace umock {

class UPNPLIB_API StdlibInterface {
  public:
    StdlibInterface();
    virtual ~StdlibInterface();
    virtual void* malloc(size_t size) = 0;
    virtual void* calloc(size_t nmemb, size_t size) = 0;
    virtual void* realloc(void* ptr, size_t size) = 0;
    virtual void free(void* ptr) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class StdlibReal : public StdlibInterface {
  public:
    StdlibReal();
    virtual ~StdlibReal() override;
    void* malloc(size_t size) override;
    void* calloc(size_t nmemb, size_t size) override;
    void* realloc(void* ptr, size_t size) override;
    void free(void* ptr) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    StdlibReal stdlib_realObj;	      // already done below
    Stdlib stdlib_h(&stdlib_realObj); // already done below
    { // Other scope, e.g. within a gtest
        class StdlibMock : public StdlibInterface { ...; MOCK_METHOD(...) };
        StdlibMock stdlib_mockObj;
        Stdlib stdlib_injectObj(&stdlib_mockObj); // obj. name doesn't matter
        EXPECT_CALL(stdlib_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Stdlib {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Stdlib(StdlibReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Stdlib(StdlibInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Stdlib();

    // Methods
    virtual void* malloc(size_t size);
    virtual void* calloc(size_t nmemb, size_t size);
    virtual void* realloc(void* ptr, size_t size);
    virtual void free(void* ptr);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Stdlib::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline StdlibInterface* m_ptr_workerObj;
    StdlibInterface* m_ptr_oldObj{};
};


UPNPLIB_EXTERN Stdlib stdlib_h;

} // namespace umock

#endif // UMOCK_STDLIB_HPP
