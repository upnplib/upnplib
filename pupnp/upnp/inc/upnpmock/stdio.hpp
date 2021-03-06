// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-17

#ifndef UPNPLIB_STDIOIF_HPP
#define UPNPLIB_STDIOIF_HPP

#include <stdio.h>

namespace upnplib {

class Bstdio {
    // Base class to call the system functions.
  public:
    virtual ~Bstdio() {}

    virtual FILE* fopen(const char* pathname, const char* mode) {
        return ::fopen(pathname, mode);
    }
    virtual int fclose(FILE* stream) { return ::fclose(stream); }
    virtual int fflush(FILE* stream) { return ::fflush(stream); }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
EXPORT_SPEC extern Bstdio* stdio_h;

// In the production code you just prefix the old system call with
// 'upnplib::stdio_h->' so the new call looks like this:
//  upnplib::stdio_h->fopen(pathname, mode)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_stdio : public Bstdio {
    // Class to mock the free system functions.
    Bstdio* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_stdio() { m_oldptr = stdio_h; stdio_h = this; }
    virtual ~Mock_stdio() { stdio_h = m_oldptr; }

    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode), (override));
    MOCK_METHOD(int, fclose, (FILE* stream), (override));
    MOCK_METHOD(int, fflush, (FILE* stream), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_stdio m_mocked_stdio;

 *  and call it with: m_mocked_stdio.fopen(pathname, mode)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_STDIOIF_HPP
