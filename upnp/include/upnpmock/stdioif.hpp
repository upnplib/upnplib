// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-10

#ifndef UPNP_STDIOIF_H
#define UPNP_STDIOIF_H

#include <stdio.h>

namespace upnp {

class Istdio {
    // Interface to stdio system calls
  public:
    virtual ~Istdio() {}
    virtual FILE* fopen(const char* pathname, const char* mode) = 0;
    virtual int fclose(FILE* stream) = 0;
    virtual int fflush(FILE* stream) = 0;
};

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
Istdio* stdioif;

class Cstdio : public Istdio {
    // Real class to call the system functions.
  public:
    virtual ~Cstdio() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cstdio() { stdioif = this; }

    FILE* fopen(const char* pathname, const char* mode) override {
        return ::fopen(pathname, mode);
    }
    int fclose(FILE* stream) override { return ::fclose(stream); }
    int fflush(FILE* stream) override { return ::fflush(stream); }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer stdioif (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cstdio stdioObj;

// In the production code you must call it with, e.g.:
// stdioif->fopen(pathname, mode)

/*
 * The following class should be coppied to the test source. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_stdio : public Istdio {
// Class to mock the free system functions.
    Istdio* m_oldptr;
  public:
    // Save and restore the old pointer to the production function
    Mock_stdio() { m_oldptr = stdioif; stdioif = this; }
    virtual ~Mock_stdio() { stdioif = m_oldptr; }

    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode), (override));
    MOCK_METHOD(int, fclose, (FILE* stream), (override));
    MOCK_METHOD(int, fflush, (FILE* stream), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_stdio mocked_stdio;

 *  and call it with: mocked_stdio.fopen(pathname, mode)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_STDIOIF_H
