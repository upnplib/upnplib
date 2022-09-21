#ifndef MOCKING_NETIO_HPP
#define MOCKING_NETIO_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-21

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#include <stdio.h>

namespace mocking {

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
    StdioReal stdio_realObj;
    Stdio(&stdio_realObj;
    { // Other scope, e.g. within a gtest
        class StdioMock { ...; MOCK_METHOD(...) };
        StdioMock stdio_mockObj;
        Stdio(&stdio_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class EXPORT_SPEC Stdio {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Stdio(StdioReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Stdio(StdioInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Stdio();

#ifdef _WIN32
    // Secure function only on MS Windows
    virtual errno_t fopen_s(FILE** pFile, const char* pathname,
                            const char* mode);
#endif
    virtual FILE* fopen(const char* pathname, const char* mode);
    virtual int fclose(FILE* stream);
    virtual int fflush(FILE* stream);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Stdio::m_ptr_workerObj.
    EXPORT_SPEC_LOCAL static inline StdioInterface* m_ptr_workerObj;
    StdioInterface* m_ptr_oldObj{};
};

extern Stdio EXPORT_SPEC stdio_h;

} // namespace mocking

#endif // MOCKING_NETIO_HPP
