// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-16

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

#include "upnplib/visibility.hpp"
#include <stdio.h>

namespace upnplib::mocking {

class StdioInterface {
  public:
    virtual ~StdioInterface() {}

#ifdef _WIN32
    // Secure function only on MS Windows, not completely virtual
    virtual errno_t fopen_s(FILE** pFile, const char* pathname,
                            const char* mode) = 0;
#endif
    // Portable function also available on MS Windows
    virtual FILE* fopen(const char* pathname, const char* mode) = 0;
    virtual int fclose(FILE* stream) = 0;
    virtual int fflush(FILE* stream) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class StdioReal : public StdioInterface {
  public:
    virtual ~StdioReal() override {}

#ifdef _WIN32
    // Secure function only on MS Windows
    errno_t fopen_s(FILE** pFile, const char* pathname,
                    const char* mode) override;
#endif
    FILE* fopen(const char* pathname, const char* mode) override;
    int fclose(FILE* stream) override;
    int fflush(FILE* stream) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    StdioReal stdio_realObj; // already done below
    Stdio(&stdio_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class StdioMock : public StdioInterface { ...; MOCK_METHOD(...) };
        StdioMock stdio_mockObj;
        Stdio stdio_injectObj(&stdio_mockObj); // obj. name doesn't matter
        EXPECT_CALL(stdio_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Stdio {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Stdio(StdioReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Stdio(StdioInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Stdio();

    // Methods
#ifdef _WIN32
    // Secure function only on MS Windows
    virtual errno_t fopen_s(FILE** pFile, const char* pathname,
                            const char* mode);
#endif
    virtual FILE* fopen(const char* pathname, const char* mode);
    virtual int fclose(FILE* stream);
    virtual int fflush(FILE* stream);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Stdio::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline StdioInterface* m_ptr_workerObj;
    StdioInterface* m_ptr_oldObj{};
};

} // namespace upnplib::mocking
