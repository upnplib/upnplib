// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-24

// There is no include guard '#ifndef ...' because this file shouldn't be
// included more than two times as given.

#include "upnplib/visibility.hpp"
#include <cstring>

/* strndup() is a GNU extension. */
#if !HAVE_STRNDUP || defined(_WIN32)
UPNPLIB_API char* strndup(const char* __string, size_t __n);
#endif

namespace umock {

class StringhInterface {
  public:
    virtual ~StringhInterface() = default;
    virtual char* strerror(int errnum) = 0;
    virtual char* strdup(const char* s) = 0;
    virtual char* strndup(const char* s, size_t n) = 0;
};

//
// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class StringhReal : public StringhInterface {
  public:
    virtual ~StringhReal() override = default;
    char* strerror(int errnum) override;
    char* strdup(const char* s) override;
    char* strndup(const char* s, size_t n) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    StringhReal stringh_realObj; // already done below
    Stringh(&stringh_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class StringhMock : public StringhInterface { ...; MOCK_METHOD(...) };
        StringhMock stringh_mockObj;
        Stringh stringh_injectObj(&string_mockObj); // obj. name doesn't matter
        EXPECT_CALL(stringh_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Stringh {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Stringh(StringhReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Stringh(StringhInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Stringh();

    // Methods
    virtual char* strerror(int errnum);
    virtual char* strdup(const char* s);
    virtual char* strndup(const char* s, size_t n);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Stringh::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline StringhInterface* m_ptr_workerObj;
    StringhInterface* m_ptr_oldObj{};
};

} // namespace umock
