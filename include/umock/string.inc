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

class StringInterface {
  public:
    virtual ~StringInterface() = default;
    virtual char* strerror(int errnum) = 0;
    virtual char* strdup(const char* s) = 0;
    virtual char* strndup(const char* s, size_t n) = 0;
};

//
// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class StringReal : public StringInterface {
  public:
    virtual ~StringReal() override = default;
    char* strerror(int errnum) override;
    char* strdup(const char* s) override;
    char* strndup(const char* s, size_t n) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    StringReal string_realObj; // already done below
    String(&string_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class StringMock : public StringInterface { ...; MOCK_METHOD(...) };
        StringMock string_mockObj;
        String string_injectObj(&string_mockObj); // obj. name doesn't matter
        EXPECT_CALL(string_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API String {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    String(StringReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    String(StringInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~String();

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
    // cannot be accessed globaly with String::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline StringInterface* m_ptr_workerObj;
    StringInterface* m_ptr_oldObj{};
};

} // namespace umock
